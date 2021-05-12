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

#include <sstream>
#include "gambatte.h"
#include "gbcpalettes.h"
#include "resamplerinfo.h"
#include "resampler.h"

#define OptionDefault(_NAME_, _PREFKEY_) @{ OEGameCoreDisplayModeNameKey : _NAME_, OEGameCoreDisplayModePrefKeyNameKey : _PREFKEY_, OEGameCoreDisplayModeStateKey : @YES, }
#define Option(_NAME_, _PREFKEY_) @{ OEGameCoreDisplayModeNameKey : _NAME_, OEGameCoreDisplayModePrefKeyNameKey : _PREFKEY_, OEGameCoreDisplayModeStateKey : @NO, }
#define OptionIndented(_NAME_, _PREFKEY_) @{ OEGameCoreDisplayModeNameKey : _NAME_, OEGameCoreDisplayModePrefKeyNameKey : _PREFKEY_, OEGameCoreDisplayModeStateKey : @NO, OEGameCoreDisplayModeIndentationLevelKey : @(1), }
#define OptionToggleable(_NAME_, _PREFKEY_) @{ OEGameCoreDisplayModeNameKey : _NAME_, OEGameCoreDisplayModePrefKeyNameKey : _PREFKEY_, OEGameCoreDisplayModeStateKey : @NO, OEGameCoreDisplayModeAllowsToggleKey : @YES, }
#define OptionToggleableNoSave(_NAME_, _PREFKEY_) @{ OEGameCoreDisplayModeNameKey : _NAME_, OEGameCoreDisplayModePrefKeyNameKey : _PREFKEY_, OEGameCoreDisplayModeStateKey : @NO, OEGameCoreDisplayModeAllowsToggleKey : @YES, OEGameCoreDisplayModeDisallowPrefSaveKey : @YES, }
#define Label(_NAME_) @{ OEGameCoreDisplayModeLabelKey : _NAME_, }
#define SeparatorItem() @{ OEGameCoreDisplayModeSeparatorItemKey : @"",}

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
    uint32_t *_videoBuffer;
    uint32_t *_inSoundBuffer;
    int16_t *_outSoundBuffer;
    double _sampleRate;
    NSMutableDictionary <NSString *, NSNumber *> *_cheatList;
    NSMutableArray <NSMutableDictionary <NSString *, id> *> *_availableDisplayModes;
}

- (void)applyCheat:(NSString *)code;

- (NSString *)gameInternalName;
- (BOOL)gameHasInternalPalette;
- (void)loadPalette;
- (void)loadPaletteDefault;
- (void)changePalette:(NSString *)palette;

@end

@implementation GBGameCore

- (id)init
{
    if((self = [super init]))
    {
        _inSoundBuffer = (uint32_t *)malloc(2064 * 2 * 4);
        _outSoundBuffer = (int16_t *)malloc(2064 * 2 * 2);
        _cheatList = [NSMutableDictionary dictionary];
    }

	return self;
}

- (void)dealloc
{
    free(_videoBuffer);
    free(_inSoundBuffer);
    free(_outSoundBuffer);
}

# pragma mark - Execution

- (BOOL)loadFileAtPath:(NSString *)path error:(NSError **)error
{
    memset(pad, 0, sizeof(uint32_t) * OEGBButtonCount);

    // Set battery save dir
    NSURL *batterySavesDirectory = [NSURL fileURLWithPath:self.batterySavesDirectoryPath];
    [[NSFileManager defaultManager] createDirectoryAtURL:batterySavesDirectory withIntermediateDirectories:YES attributes:nil error:nil];
    gb.setSaveDir(batterySavesDirectory.fileSystemRepresentation);

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
    _sampleRate = outSampleRate; // 47994.326636

    if (gb.load(path.fileSystemRepresentation) != 0)
        return NO;

    // Load built-in GBC palette for monochrome games if supported
    if (!gb.isCgb())
        [self loadPalette];

    return YES;
}

- (void)executeFrame
{
    size_t samples = 2064;

    while (gb.runFor(_videoBuffer, 160, _inSoundBuffer, samples) == -1) {
        [self outputAudio:samples];
        samples = 2064;
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

    delete resampler;

    [super stopEmulation];
}

- (NSTimeInterval)frameInterval
{
    return 59.727501;
}

# pragma mark - Video

- (const void *)getVideoBufferWithHint:(void *)hint
{
    if (!hint) {
        if (!_videoBuffer) _videoBuffer = (uint32_t *)malloc(160 * 144 * 4);
        hint = _videoBuffer;
    }
    return _videoBuffer = (uint32_t*)hint;
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

# pragma mark - Audio

- (double)audioSampleRate
{
    return _sampleRate;
}

- (NSUInteger)channelCount
{
    return 2;
}

# pragma mark - Save States

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    int success = gb.saveState(0, 0, fileName.fileSystemRepresentation);
    if(block) block(success==1, nil);
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    int success = gb.loadState(fileName.fileSystemRepresentation);
    if(block) block(success==1, nil);
}

- (NSData *)serializeStateWithError:(NSError **)outError
{
    std::stringstream stream(std::ios::in|std::ios::out|std::ios::binary);

    if(gb.serializeState(stream)) {
        stream.seekg(0, std::ios::end);
        NSUInteger length = stream.tellg();
        stream.seekg(0, std::ios::beg);

        char *bytes = (char *)malloc(length);
        stream.read(bytes, length);

        return [NSData dataWithBytesNoCopy:bytes length:length];
    }

    if(outError) {
        *outError = [NSError errorWithDomain:OEGameCoreErrorDomain code:OEGameCoreCouldNotSaveStateError userInfo:@{
            NSLocalizedDescriptionKey : @"Save state data could not be written",
            NSLocalizedRecoverySuggestionErrorKey : @"The emulator could not write the state data."
        }];
    }

    return nil;
}

- (BOOL)deserializeState:(NSData *)state withError:(NSError **)outError
{
    std::stringstream stream(std::ios::in|std::ios::out|std::ios::binary);

    char const *bytes = (char const *)(state.bytes);
    std::streamsize size = state.length;
    stream.write(bytes, size);

    if(gb.deserializeState(stream))
        return YES;

    if(outError) {
        *outError = [NSError errorWithDomain:OEGameCoreErrorDomain code:OEGameCoreCouldNotLoadStateError userInfo:@{
            NSLocalizedDescriptionKey : @"The save state data could not be read",
            NSLocalizedRecoverySuggestionErrorKey : @"Could not load data from the save state"
        }];
    }
    return NO;
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

- (void)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled
{
    // Sanitize
    code = [code stringByTrimmingCharactersInSet:NSCharacterSet.whitespaceAndNewlineCharacterSet];

    // Gambatte expects cheats UPPERCASE
    code = code.uppercaseString;

    // Remove any spaces
    code = [code stringByReplacingOccurrencesOfString:@" " withString:@""];

    if (enabled)
        _cheatList[code] = @YES;
    else
        [_cheatList removeObjectForKey:code];

    NSMutableArray <NSString *> *combinedGameSharkCodes = [NSMutableArray array];
    NSMutableArray <NSString *> *combinedGameGenieCodes = [NSMutableArray array];

    // Gambatte expects all cheats in one combined string per-type e.g. 01xxxxxx+01xxxxxx
    // Add enabled per-type cheats to arrays and later join them all by a '+' separator
    for (NSString *key in _cheatList)
    {
        if ([_cheatList[key] boolValue])
        {
            // GameShark
            if (![key containsString:@"-"])
                [combinedGameSharkCodes addObject:key];
            // Game Genie
            else if ([key containsString:@"-"])
                [combinedGameGenieCodes addObject:key];
        }
    }

    // Apply combined cheats or force a final reset if all cheats are disabled
    [self applyCheat:combinedGameSharkCodes.count != 0 ? [combinedGameSharkCodes componentsJoinedByString:@"+"] : @"0"];
    [self applyCheat:combinedGameGenieCodes.count != 0 ? [combinedGameGenieCodes componentsJoinedByString:@"+"] : @"0-"];
}

# pragma mark - Display Mode

- (NSArray <NSDictionary <NSString *, id> *> *)displayModes
{
    if (gb.isCgb())
        return nil;

    if (_availableDisplayModes.count == 0)
    {
        _availableDisplayModes = [NSMutableArray array];

        NSArray <NSDictionary <NSString *, id> *> *availableModesWithDefault =
        @[
          Option(@"Optimal Palette", @"palette"),
          SeparatorItem(),
          Label(@"Internal Palettes"),
          Option(@"Super Game Boy", @"palette"),
          Option(@"Game Boy Color", @"palette"),
          SeparatorItem(),
          Label(@"Hardware Palettes"),
          Option(@"Dot Matrix", @"palette"),
          Option(@"Game Boy Pocket", @"palette"),
          Option(@"Game Boy Light", @"palette"),
          SeparatorItem(),
          Label(@"Monochrome Palettes"),
          Option(@"SGB Greenscale", @"palette"),
          Option(@"SGB Grayscale", @"palette"),
          Option(@"GBC Grayscale", @"palette"),
          SeparatorItem(),
          Label(@"Virtual Console"),
          Option(@"VC Greenscale", @"palette"),
          Option(@"VC Grayscale", @"palette"),
          SeparatorItem(),
          Label(@"Extra Palettes"),
          Option(@"Analogue", @"palette"),
          Option(@"Greenscale", @"palette"),
          Option(@"Pocket Tales", @"palette"),
          Option(@"VirtualBoy", @"palette"),
          Option(@"WonderSwan", @"palette")
        ];

        // Deep mutable copy
        _availableDisplayModes = (NSMutableArray *)CFBridgingRelease(CFPropertyListCreateDeepCopy(kCFAllocatorDefault, (CFArrayRef)availableModesWithDefault, kCFPropertyListMutableContainers));
    }

    return [_availableDisplayModes copy];
}

- (void)changeDisplayWithMode:(NSString *)displayMode
{
    // NOTE: This is a more complex implementation to serve as an example for handling submenus,
    // toggleable options and multiple groups of mutually exclusive options.

    if (_availableDisplayModes.count == 0)
        [self displayModes];

    // First check if 'displayMode' is toggleable and grab its preference key
    BOOL isDisplayModeToggleable = NO;
    BOOL isValidDisplayMode = NO;
    BOOL displayModeState = NO;
    NSString *displayModePrefKey;

    for (NSDictionary *modeDict in _availableDisplayModes)
    {
        if ([modeDict[OEGameCoreDisplayModeNameKey] isEqualToString:displayMode])
        {
            displayModeState = [modeDict[OEGameCoreDisplayModeStateKey] boolValue];
            displayModePrefKey = modeDict[OEGameCoreDisplayModePrefKeyNameKey];
            isDisplayModeToggleable = [modeDict[OEGameCoreDisplayModeAllowsToggleKey] boolValue];
            isValidDisplayMode = YES;
            break;
        }
        // Submenu Items
        for (NSDictionary *subModeDict in modeDict[OEGameCoreDisplayModeGroupItemsKey])
        {
            if ([subModeDict[OEGameCoreDisplayModeNameKey] isEqualToString:displayMode])
            {
                displayModeState = [subModeDict[OEGameCoreDisplayModeStateKey] boolValue];
                displayModePrefKey = subModeDict[OEGameCoreDisplayModePrefKeyNameKey];
                isDisplayModeToggleable = [subModeDict[OEGameCoreDisplayModeAllowsToggleKey] boolValue];
                isValidDisplayMode = YES;
                break;
            }
        }
    }

    // Disallow a 'displayMode' not found in _availableDisplayModes
    if (!isValidDisplayMode)
        return;

    // Handle option state changes
    for (NSMutableDictionary *optionDict in _availableDisplayModes)
    {
        NSString *modeName = optionDict[OEGameCoreDisplayModeNameKey];
        NSString *prefKey  = optionDict[OEGameCoreDisplayModePrefKeyNameKey];

        if (!modeName && !optionDict[OEGameCoreDisplayModeGroupNameKey])
            continue;
        // Mutually exclusive option state change
        else if ([modeName isEqualToString:displayMode] && !isDisplayModeToggleable)
            optionDict[OEGameCoreDisplayModeStateKey] = @YES;
        // Reset mutually exclusive options that are the same prefs group as 'displayMode'
        else if (!isDisplayModeToggleable && [prefKey isEqualToString:displayModePrefKey])
            optionDict[OEGameCoreDisplayModeStateKey] = @NO;
        // Toggleable option state change
        else if ([modeName isEqualToString:displayMode] && isDisplayModeToggleable)
            optionDict[OEGameCoreDisplayModeStateKey] = @(!displayModeState);
        // Submenu group
        else if (optionDict[OEGameCoreDisplayModeGroupNameKey])
        {
            // Submenu items
            for (NSMutableDictionary *subOptionDict in optionDict[OEGameCoreDisplayModeGroupItemsKey])
            {
                NSString *modeName = subOptionDict[OEGameCoreDisplayModeNameKey];
                NSString *prefKey  = subOptionDict[OEGameCoreDisplayModePrefKeyNameKey];

                if (!modeName)
                    continue;
                // Mutually exclusive option state change
                else if ([modeName isEqualToString:displayMode] && !isDisplayModeToggleable)
                    subOptionDict[OEGameCoreDisplayModeStateKey] = @YES;
                // Reset mutually exclusive options that are the same prefs group as 'displayMode'
                else if (!isDisplayModeToggleable && [prefKey isEqualToString:displayModePrefKey])
                    subOptionDict[OEGameCoreDisplayModeStateKey] = @NO;
                // Toggleable option state change
                else if ([modeName isEqualToString:displayMode] && isDisplayModeToggleable)
                    subOptionDict[OEGameCoreDisplayModeStateKey] = @(!displayModeState);
            }

            continue;
        }
    }

    // Set the new palette
    if ([displayModePrefKey isEqualToString:@"palette"])
        [self changePalette:displayMode];
}

# pragma mark - Misc Helper Methods

- (void)outputAudio:(size_t)frames
{
    if (!frames)
        return;

    size_t len = resampler->resample(_outSoundBuffer, reinterpret_cast<const int16_t *>(_inSoundBuffer), frames);

    if (len)
        [[self ringBufferAtIndex:0] write:_outSoundBuffer maxLength:len << 2];
}

- (void)applyCheat:(NSString *)code
{
    std::string s = [code UTF8String];
    if (s.find("-") != std::string::npos)
        gb.setGameGenie(s);
    else
        gb.setGameShark(s);
}

- (NSString *)gameInternalName
{
    NSString *title = [NSString stringWithUTF8String:gb.romTitle().c_str()];
    return title;
}

- (BOOL)gameHasInternalPalette
{
    unsigned short *gbc_bios_palette = NULL;
    NSString *title = [self gameInternalName];
    gbc_bios_palette = const_cast<unsigned short *>(findGbcTitlePal(title.UTF8String));

    return gbc_bios_palette != 0 ? YES : NO;
}

- (void)loadPalette
{
    // Only temporary, so core doesn't crash on an older OpenEmu version
    if (![self respondsToSelector:@selector(displayModeInfo)])
    {
        [self loadPaletteDefault];
    }
    // No previous palette saved, set a default
    else if (self.displayModeInfo[@"palette"] == nil)
    {
        [self loadPaletteDefault];
    }
    else
    {
        NSString *lastPalette = self.displayModeInfo[@"palette"];
        [self changeDisplayWithMode:lastPalette];
    }
}

- (void)loadPaletteDefault
{
    [self changeDisplayWithMode:@"Optimal Palette"];
}

- (void)changePalette:(NSString *)palette
{
    NSDictionary <NSString *, NSString *> *paletteNames =
    @{
      @"Optimal Palette"  : @"Optimal",
      @"Super Game Boy"   : @"SGB",
      @"Game Boy Color"   : @"GBC",
      @"Dot Matrix"       : @"GB - DMG",
      @"Game Boy Pocket"  : @"GB - MGB",
      @"Game Boy Light"   : @"GB - Light",
      @"SGB Greenscale"   : @"SGB - 4H",
      @"SGB Grayscale"    : @"SGB - 2H",
      @"GBC Grayscale"    : @"GBC - Grayscale",
      @"VirtualBoy"       : @"VirtualBoy",
      @"VC Greenscale"    : @"VC - Green",
      @"VC Grayscale"     : @"VC - Gray",
      @"Analogue"         : @"Analogue",
      @"Greenscale"       : @"Squidlit",
      @"Pocket Tales"     : @"Pocket Tales",
      @"VirtualBoy"       : @"VirtualBoy",
      @"WonderSwan"       : @"WonderSwan"
    };

    palette = paletteNames[palette];
    unsigned short *gbc_bios_palette = NULL;
    unsigned short *sgb_bios_palette = NULL;

    if ([palette isEqualToString:@"Optimal"])
    {
        NSString *title = [self gameInternalName];

        gbc_bios_palette = const_cast<unsigned short *>(findGbcTitlePal(title.UTF8String)); // load a GBC BIOS built-in palette
        sgb_bios_palette = const_cast<unsigned short *>(findSgbTitlePal(title.UTF8String)); // load a SGB BIOS built-in palette

        // if both GBC and SGB palettes are defined, use whichever is more colourful
        if (gbc_bios_palette != 0)
        {
          if (sgb_bios_palette != 0)
          {
            if ((gbc_bios_palette == p005) || (gbc_bios_palette == p006) || (gbc_bios_palette == p007) || (gbc_bios_palette == p008) || (gbc_bios_palette == p012) || (gbc_bios_palette == p013) || (gbc_bios_palette == p016) || (gbc_bios_palette == p017) || (gbc_bios_palette == p01B))
            {
              // if a limited color GBC palette, use SGB equivalent
              gbc_bios_palette = sgb_bios_palette;
            }
          }
        }

        if (gbc_bios_palette == 0)
        {
            // no custom GBC palette found, load the SGB BIOS builtin palette
            gbc_bios_palette = sgb_bios_palette;
        }

        if (gbc_bios_palette == 0)
        {
            // no custom palette found, load GB Pocket inspired TV palette from Kirby's Dream Collection
            // NOTE: I found traditional grayscale with a white background is a bit blinding
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("VC - Gray"));
        }
    }
    else if ([palette isEqualToString:@"SGB"])
    {
        NSString *title = [self gameInternalName];

        gbc_bios_palette = const_cast<unsigned short *>(findSgbTitlePal(title.UTF8String)); // load a SGB BIOS built-in palette

        if (gbc_bios_palette == 0)
        {
            // no custom palette found, load SGB Default 1-A
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("SGB - 1A"));
        }
    }
    else if ([palette isEqualToString:@"GBC"])
    {
        NSString *title = [self gameInternalName];

        gbc_bios_palette = const_cast<unsigned short *>(findGbcTitlePal(title.UTF8String)); // load a GBC BIOS built-in palette

        if (gbc_bios_palette == 0)
        {
            // no custom palette found, load GBC Default
            gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal("GBC - Dark Green"));
        }
    }
    else if ([palette isEqualToString:@"Greenscale"])
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
        return;
    }
    else if ([palette isEqualToString:@"Pocket"])
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
        return;
    }
    else
        gbc_bios_palette = const_cast<unsigned short *>(findGbcDirPal(palette.UTF8String));

    unsigned long rgb32 = 0;
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
