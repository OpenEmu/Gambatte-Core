/*
 Copyright (c) 2015, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "GBGameCore.h"
#import <OpenEmuBase/OERingBuffer.h>
#import "OEGBSystemResponderClient.h"
#import <OpenGL/gl.h>

#include "gambatte.h"
#include "gbcpalettes.h"
#include "resamplerinfo.h"
#include "resampler.h"

gambatte::GB gb;
Resampler *resampler;
uint32_t pad[OEGBButtonCount];

class GetInput : public gambatte::InputGetter
{
public:
    unsigned operator()()
    {
        return pad[0];
    }
} static GetInput;

@interface GBGameCore () <OEGBSystemResponderClient>
{
    uint32_t *videoBuffer;
    uint32_t *inSoundBuffer;
    int16_t *outSoundBuffer;
    double sampleRate;
    int displayMode;
}
- (void)outputAudio:(unsigned)frames;
- (void)applyCheat:(NSString *)code;
- (void)loadPalette;
@end

@implementation GBGameCore

static __weak GBGameCore *_current;

- (id)init
{
    if((self = [super init]))
    {
        videoBuffer = (uint32_t *)malloc(160 * 144 * 4);
        inSoundBuffer = (uint32_t *)malloc(2064 * 2);
        outSoundBuffer = (int16_t *)malloc(2064 * 2);
        displayMode = 0;
    }

	_current = self;

	return self;
}

- (void)dealloc
{
    free(videoBuffer);
    free(inSoundBuffer);
    free(outSoundBuffer);
}

# pragma mark - Execution

- (BOOL)loadFileAtPath:(NSString *)path error:(NSError **)error
{
    memset(pad, 0, sizeof(uint32_t) * OEGBButtonCount);

    // Set battery save dir
    NSURL *batterySavesDirectory = [NSURL fileURLWithPath:[self batterySavesDirectoryPath]];
    [[NSFileManager defaultManager] createDirectoryAtURL:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:nil];
    gb.setSaveDir([[batterySavesDirectory path] UTF8String]);

    // Set input state callback
    gb.setInputGetter(&GetInput);

    // Setup resampler
    double fps = 4194304.0 / 70224.0;
    double inSampleRate = fps * 35112; // 2097152

    // 2 = "Very high quality (polyphase FIR)", see resamplerinfo.cpp
    resampler = ResamplerInfo::get(2).create(inSampleRate, 48000.0, 2 * 2064);

    unsigned long mul, div;
    resampler->exactRatio(mul, div);

    double outSampleRate = inSampleRate * mul / div;
    sampleRate = outSampleRate; // 47994.326636

    if (gb.load([path UTF8String]) != 0)
        return NO;

    // Load built-in GBC palette for monochrome games if supported
    if (!gb.isCgb())
        [self loadPalette];

    return YES;
}

- (void)executeFrame
{
    [self executeFrameSkippingFrame:NO];
}

- (void)executeFrameSkippingFrame:(BOOL)skip
{
    size_t samples = 2064;

    while (gb.runFor(videoBuffer, 160, inSoundBuffer, samples) == -1)
    {
        [self outputAudio:samples];
    }

    [self outputAudio:samples];
}

- (void)resetEmulation
{
    gb.reset();
}

- (void)stopEmulation
{
    gb.saveSavedata();

    //delete resampler;

    [super stopEmulation];
}

- (NSTimeInterval)frameInterval
{
    return 59.727501;
}

# pragma mark - Video

- (const void *)videoBuffer
{
    return videoBuffer;
}

- (OEIntRect)screenRect
{
    return OEIntRectMake(0, 0, 160, 144);
}

- (OEIntSize)bufferSize
{
    return OEIntSizeMake(160, 144);
}

- (OEIntSize)aspectSize
{
    return OEIntSizeMake(10, 9);
}

- (GLenum)pixelFormat
{
    return GL_BGRA;
}

- (GLenum)pixelType
{
    return GL_UNSIGNED_INT_8_8_8_8_REV;
}

- (GLenum)internalPixelFormat
{
    return GL_RGB8;
}

# pragma mark - Audio

- (double)audioSampleRate
{
    return sampleRate;
}

- (NSUInteger)channelCount
{
    return 2;
}

# pragma mark - Save States

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    int success = gb.saveState(0, 0, [fileName UTF8String]);
    if(block) block(success==1, nil);
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    int success = gb.loadState([fileName UTF8String]);
    if(block) block(success==1, nil);
}

# pragma mark - Input

const int GBMap[] = {gambatte::InputGetter::UP, gambatte::InputGetter::DOWN, gambatte::InputGetter::LEFT, gambatte::InputGetter::RIGHT, gambatte::InputGetter::A, gambatte::InputGetter::B, gambatte::InputGetter::START, gambatte::InputGetter::SELECT};
- (oneway void)didPushGBButton:(OEGBButton)button;
{
    pad[0] |= GBMap[button];
}

- (oneway void)didReleaseGBButton:(OEGBButton)button;
{
    pad[0] &= ~GBMap[button];
}

#pragma mark - Cheats

NSMutableDictionary *cheatList = [[NSMutableDictionary alloc] init];

- (void)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled
{
    // Sanitize
    code = [code stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

    // Gambatte expects cheats UPPERCASE
    code = [code uppercaseString];

    // Remove any spaces
    code = [code stringByReplacingOccurrencesOfString:@" " withString:@""];

    if (enabled)
        [cheatList setValue:@YES forKey:code];
    else
        [cheatList removeObjectForKey:code];

    NSMutableArray *combinedGameSharkCodes = [[NSMutableArray alloc] init];
    NSMutableArray *combinedGameGenieCodes = [[NSMutableArray alloc] init];

    // Gambatte expects all cheats in one combined string per-type e.g. 01xxxxxx+01xxxxxx
    // Add enabled per-type cheats to arrays and later join them all by a '+' separator
    for (id key in cheatList)
    {
        if ([[cheatList valueForKey:key] isEqual:@YES])
        {
            // GameShark
            if ([key rangeOfString:@"-"].location == NSNotFound)
                [combinedGameSharkCodes addObject:key];
            // Game Genie
            else if ([key rangeOfString:@"-"].location != NSNotFound)
                [combinedGameGenieCodes addObject:key];
        }
    }

    // Apply combined cheats or force a final reset if all cheats are disabled
    [self applyCheat:[combinedGameSharkCodes count] != 0 ? [combinedGameSharkCodes componentsJoinedByString:@"+"] : @"0"];
    [self applyCheat:[combinedGameGenieCodes count] != 0 ? [combinedGameGenieCodes componentsJoinedByString:@"+"] : @"0-"];
}

# pragma mark - Display Mode

- (void)changeDisplayMode
{
    if (gb.isCgb())
        return;

    unsigned short *gbc_bios_palette = NULL;

    switch (displayMode)
    {
        case 0:
        {
            // GB Pea Soup Green
            gb.setDmgPaletteColor(0, 0, 8369468);
            gb.setDmgPaletteColor(0, 1, 6728764);
            gb.setDmgPaletteColor(0, 2, 3629872);
            gb.setDmgPaletteColor(0, 3, 3223857);
            gb.setDmgPaletteColor(1, 0, 8369468);
            gb.setDmgPaletteColor(1, 1, 6728764);
            gb.setDmgPaletteColor(1, 2, 3629872);
            gb.setDmgPaletteColor(1, 3, 3223857);
            gb.setDmgPaletteColor(2, 0, 8369468);
            gb.setDmgPaletteColor(2, 1, 6728764);
            gb.setDmgPaletteColor(2, 2, 3629872);
            gb.setDmgPaletteColor(2, 3, 3223857);

            displayMode++;
            return;
        }

        case 1:
        {
            // GB Pocket
            gb.setDmgPaletteColor(0, 0, 13487791);
            gb.setDmgPaletteColor(0, 1, 10987158);
            gb.setDmgPaletteColor(0, 2, 6974033);
            gb.setDmgPaletteColor(0, 3, 2828823);
            gb.setDmgPaletteColor(1, 0, 13487791);
            gb.setDmgPaletteColor(1, 1, 10987158);
            gb.setDmgPaletteColor(1, 2, 6974033);
            gb.setDmgPaletteColor(1, 3, 2828823);
            gb.setDmgPaletteColor(2, 0, 13487791);
            gb.setDmgPaletteColor(2, 1, 10987158);
            gb.setDmgPaletteColor(2, 2, 6974033);
            gb.setDmgPaletteColor(2, 3, 2828823);

            displayMode++;
            return;
        }

        case 2:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Blue"));
            displayMode++;
            break;

        case 3:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Dark Blue"));
            displayMode++;
            break;

        case 4:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Green"));
            displayMode++;
            break;

        case 5:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Dark Green"));
            displayMode++;
            break;

        case 6:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Brown"));
            displayMode++;
            break;

        case 7:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Dark Brown"));
            displayMode++;
            break;

        case 8:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Red"));
            displayMode++;
            break;

        case 9:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Yellow"));
            displayMode++;
            break;

        case 10:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Orange"));
            displayMode++;
            break;

        case 11:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Pastel Mix"));
            displayMode++;
            break;

        case 12:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Inverted"));
            displayMode++;
            break;

        case 13:
        {
            std::string str = gb.romTitle(); // read ROM internal title
            const char *internal_game_name = str.c_str();
            gbc_bios_palette = const_cast<unsigned short *>(findGbcTitlePal(internal_game_name));

            if (gbc_bios_palette == 0)
            {
                gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Grayscale"));
                displayMode = 0;
            }
            else
                displayMode++;

            break;
        }

        case 14:
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Grayscale"));
            displayMode = 0;
            break;

        default:
            return;
            break;
    }

    unsigned rgb32 = 0;
    for (unsigned palnum = 0; palnum < 3; ++palnum)
    {
        for (unsigned colornum = 0; colornum < 4; ++colornum)
        {
            rgb32 = gbcToRgb32(gbc_bios_palette[palnum * 4 + colornum]);
            gb.setDmgPaletteColor(palnum, colornum, rgb32);
        }
    }
}

# pragma mark - Misc Helper Methods

- (void)outputAudio:(unsigned)frames
{
    if (!frames)
        return;

    size_t len = resampler->resample(outSoundBuffer, reinterpret_cast<const int16_t *>(inSoundBuffer), frames);

    if (len)
        [[self ringBufferAtIndex:0] write:outSoundBuffer maxLength:len << 2];
}

- (void)applyCheat:(NSString *)code
{
    std::string s = [code UTF8String];
    if (s.find("-") != std::string::npos)
        gb.setGameGenie(s);
    else
        gb.setGameShark(s);
}

- (void)loadPalette
{
    std::string str = gb.romTitle(); // read ROM internal title
    const char *internal_game_name = str.c_str();

    // load a GBC BIOS builtin palette
    unsigned short *gbc_bios_palette = NULL;
    gbc_bios_palette = const_cast<unsigned short *>(findGbcTitlePal(internal_game_name));

    if (gbc_bios_palette == 0)
    {
        // no custom palette found, load the default (Original Grayscale)
        gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Grayscale"));
    }

    unsigned rgb32 = 0;
    for (unsigned palnum = 0; palnum < 3; ++palnum)
    {
        for (unsigned colornum = 0; colornum < 4; ++colornum)
        {
            rgb32 = gbcToRgb32(gbc_bios_palette[palnum * 4 + colornum]);
            gb.setDmgPaletteColor(palnum, colornum, rgb32);
        }
    }
}

@end
