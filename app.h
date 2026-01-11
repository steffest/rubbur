#include <exec/memory.h>
#include "clib/graphics_protos.h"

#define SCREENWIDTH 320
#define SCREENHEIGHT 256
#define SCREENDEPTH 5
#define PLANESIZE (SCREENWIDTH * SCREENHEIGHT / 8)
#define WRITE_DEBUG 0
#define WRITE_DEBUG_DETAIL 0

#define kBitsPerByte 0x8
#define kBitsPerWord 0x10
#define kBytesPerWord 0x2
#define kUWordMax 0xFFFF

#define ASSET_TYPE_NONE 0
#define ASSET_TYPE_LOADED 1
#define ASSET_TYPE_COMPRESSED 2

#include "framework/amiga_custom.h"
#include "framework/system.h"
#include "framework/input.h"
#include "framework/screen.h"
#include "framework/bitplane.h"
#include "framework/assetManager.h"
#include "framework/display.h"
#include "framework/modPlay.h"
#include "framework/copper.h"
#include "framework/sprite.h"
#include "framework/blitter.h"
#include "framework/palette.h"
#include "framework/math.h"
#include "framework/loader.h"
#include "framework/effect_screenzoom.h"

#include "parts/parts.h"
