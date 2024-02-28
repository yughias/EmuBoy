#include "bootrom_skip.h"
#include "info.h"
#include "hardware.h"

#include "string.h"

typedef struct {
    uint8_t hash;
    char forth_char;
    int bg[4];
    int obj0[4];
    int obj1[4];
} paletteConfig;

paletteConfig paletteConfigs[] = {
    {0x00, 0, {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xB3, 'U', {0xFFFFFF, 0xADAD84, 0x42737B, 0x000000}, {0xFFFFFF, 0xFF7300, 0x944200, 0x000000}, {0xFFFFFF, 0xFF7300, 0x944200, 0x000000}},
    {0x59, 0, {0xFFFFFF, 0xADAD84, 0x42737B, 0x000000}, {0xFFFFFF, 0xFF7300, 0x944200, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0xC6, 'A', {0xFFFFFF, 0xADAD84, 0x42737B, 0x000000}, {0xFFFFFF, 0xFF7300, 0x944200, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x8C, 0, {0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A}, {0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A}, {0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A}},
    {0x86, 0, {0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xA8, 0, {0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xBF, 'C', {0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000}, {0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0xCE, 0, {0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000}, {0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0xD1, 0, {0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000}, {0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0xF0, 0, {0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000}, {0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}}, 
    {0x36, 0, {0x52DE00, 0xFF8400, 0xFFFF00, 0xFFFFFF}, {0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x34, 0, {0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x66, 'E', {0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xF4, ' ', {0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x3D, 0, {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x6A, 'I', {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x95, 0, {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0xB3, 'R', {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x71, 0, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}},
    {0xFF, 0, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}},
    {0x19, 0, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x3E, 0, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0xE0, 0, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x15, 0, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}},
    {0xDB, 0, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}},
    {0x0D, 'R', {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x69, 0, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0xF2, 0, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x88, 0, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}},
    {0x1D, 0, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}},
    {0x27, 'B', {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}, {0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF}},
    {0x49, 0, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}, {0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF}},
    {0x5C, 0, {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}, {0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF}},
    {0xB3, 'B', {0xA59CFF, 0xFFFF00, 0x006300, 0x000000}, {0xFF6352, 0xD60000, 0x630000, 0x000000}, {0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF}},
    {0xC9, 0, {0xFFFFCE, 0x63EFEF, 0x9C8431, 0x5A5A5A}, {0xFFFFFF, 0xFF7300, 0x944200, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x46, 'E', {0xB5B5FF, 0xFFFF94, 0xAD5A42, 0x000000}, {0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A}, {0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A}},
    {0x61, 'E', {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x3C, 0, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x4E, 0, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFFFF7B, 0x0084FF, 0xFF0000}},
    {0x9C, 0, {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}},
    {0x0D, 'E', {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}},
    {0x18, 'K', {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x6A, 'K', {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0x6B, 0, {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFC542, 0xFFD600, 0x943A00, 0x4A0000}, {0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF}},
    {0xD3, 'R', {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}},
    {0xBF, ' ', {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x9D, 0, {0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x28, 'F', {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x4B, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x90, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x9A, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xBD, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x17, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x27, 'N', {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x61, 'A', {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x8B, 0, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x39, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x43, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x97, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x01, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x10, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x29, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x52, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x5D, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x68, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x6D, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0xF6, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0x14, 0, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x70, 0, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x00FF00, 0x318400, 0x004A00}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x0C, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x16, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x35, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x67, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x75, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x92, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0x99, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0xB7, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}},
    {0xA5, 'R', {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0xA2, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0xF7, 0, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x28, 'A', {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}},
    {0xA5, 'A', {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}},
    {0xE8, 0, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}, {0x000000, 0x008484, 0xFFDE00, 0xFFFFFF}},
    {0x46, 'R', {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}, {0xFFFF00, 0xFF0000, 0x630000, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x008400, 0x000000}},
    {0xD3, 'I', {0xFFFFFF, 0xADAD84, 0x42737B, 0x000000}, {0xFFFFFF, 0xFFAD63, 0x843100, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}},
    {0x58, 0, {0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000}, {0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000}, {0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000}},
    {0x6F, 0,  {0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000}, {0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000}, {0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000}},
    {0xAA, 0, {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}},
    {0x18, 'I', {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x3F, 0, {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0x66, 'L', {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xC6, ' ', {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}},
    {0xF4, '-', {0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000}, {0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000}, {0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000}}
};

uint8_t getCgbTitleHash(const char* rom_name);

void skipDmgBootrom(){
    cpu.AF = 0x01B0;
    cpu.BC = 0x0013;
    cpu.DE = 0x00D8;
    cpu.HL = 0x014D;
    cpu.SP = 0xFFFE;
    cpu.PC = 0x0100;
    
    cpu.Z_FLAG = true;
    cpu.N_FLAG = false;
    cpu.H_FLAG = true;
    cpu.C_FLAG = true;

    cpu.HALTED = false;
    cpu.IME = false;
    cpu.EI_DELAY = false;
    
    SB_REG = 0x00;
    SC_REG = 0x7E;
    gb_timer.div = 0xAB;
    TIMA_REG = 0x00;
    TMA_REG = 0x00;
    TAC_REG = 0xF8;
    cpu.IF = 0xE1;
    NR10_REG = 0x80;
    NR11_REG = 0xBF;
    NR12_REG = 0xF3;
    NR13_REG = 0xFF;
    NR14_REG = 0xBF;
    NR21_REG = 0x3F;
    NR22_REG = 0x00;
    NR23_REG = 0xFF;
    NR24_REG = 0xBF;
    NR30_REG = 0x7F;
    NR31_REG = 0xFF;
    NR32_REG = 0x9F;
    NR33_REG = 0xFF;
    NR34_REG = 0xBF;
    NR41_REG = 0xFF;
    NR42_REG = 0x00;
    NR43_REG = 0x00;
    NR44_REG = 0xBF;
    NR50_REG = 0x77;
    NR51_REG = 0xF3;
    NR52_REG = 0xF1;
    LCDC_REG = 0x91;
    STAT_REG = 0x85;
    SCY_REG	= 0x00;
    SCX_REG	= 0x00;
    LY_REG	= 0x00;
    LYC_REG	= 0x00;
    DMA_REG	= 0xFF;
    BGP_REG = 0xFC;
    OBP0_REG = 0x00;
    OBP1_REG = 0x00;
    WY_REG = 0x00;
    WX_REG = 0x00;
    cpu.IE = 0x00;
}

void skipCgbBootrom(){
    cpu.A = 0x11;
    cpu.BC = 0x0000;
    cpu.DE = 0x0008;
    cpu.HL = 0x0000;
    cpu.SP = 0xFFFE;
    cpu.PC = 0x0100;
    
    cpu.Z_FLAG = true;
    cpu.N_FLAG = false;
    cpu.H_FLAG = false;
    cpu.C_FLAG = false;

    cpu.HALTED = false;
    cpu.IME = false;
    cpu.EI_DELAY = false;

    JOYP_REG = 0xC7;
    SB_REG = 0x00;
    SC_REG = 0x7E;
    gb_timer.div = 0x00;
    TIMA_REG = 0x00;
    TMA_REG = 0x00;
    TAC_REG = 0xF8;
    cpu.IF = 0xE1;
    NR10_REG = 0x80;
    NR11_REG = 0xBF;
    NR12_REG = 0xF3;
    NR13_REG = 0xFF;
    NR14_REG = 0xBF;
    NR21_REG = 0x3F;
    NR22_REG = 0x00;
    NR23_REG = 0xFF;
    NR24_REG = 0xBF;
    NR30_REG = 0x7F;
    NR31_REG = 0xFF;
    NR32_REG = 0x9F;
    NR33_REG = 0xFF;
    NR34_REG = 0xBF;
    NR41_REG = 0xFF;
    NR42_REG = 0x00;
    NR43_REG = 0x00;
    NR44_REG = 0xBF;
    NR50_REG = 0x77;
    NR51_REG = 0xF3;
    NR52_REG = 0xF0;
    LCDC_REG = 0x91;
    STAT_REG = 0x00;
    SCY_REG = 0x00;
    SCX_REG = 0x00;
    LY_REG = 0x00;
    LYC_REG = 0x00;
    DMA_REG = 0xFF;
    BGP_REG = 0xFC;
    OBP0_REG = 0x00;
    OBP1_REG = 0x00;
    WY_REG = 0x00;
    WX_REG = 0x00;
    KEY1_REG = 0x7E;
    VBK_REG = 0x00;
    HDMA_REGS[0] = 0xFF;
    HDMA_REGS[1] = 0xFF;
    HDMA_REGS[2] = 0xFF;
    HDMA_REGS[3] = 0xFF;
    HDMA_REGS[4] = 0xFF;
    BCPS_REG = 0x00;
    OCPS_REG = 0x00;
    SVBK_REG = 0x00;
    cpu.IE = 0x00;

    if(ROM[0x143] < 0x80){
        hleDmgColorization(ROM);
        console_type = DMG_ON_CGB_TYPE;
    } else {
        copyDefaultCgbPalette();
    }
}

uint8_t getCgbTitleHash(const char* rom_name){
    uint8_t hash = 0;
    while(*rom_name){
        hash += *rom_name;
        rom_name++;
    }
    return hash;
}

void hleDmgColorization(uint8_t* rom){
    const char* rom_name = getRomName(rom);
    uint8_t hash = getCgbTitleHash(rom_name);
    char forth_char = rom_name[3];
    const char* manufact_name = getManufacturerName(rom);

    if(!strcmp(manufact_name, "Nintendo") || !strcmp(manufact_name, "Nintendo R&D1"))
        for(int i = 0; i < sizeof(paletteConfigs)/sizeof(paletteConfig); i++){
            paletteConfig* config = &paletteConfigs[i];
            if(config->hash == hash && (config->forth_char == forth_char || !config->forth_char)){
                for(int j = 0; j < 4; j++){
                    int bg_col = config->bg[j];
                    int obj0_col = config->obj0[j];
                    int obj1_col = config->obj1[j];

                    writeColorToCRAM((bg_col >> 16) & 0xFF, (bg_col >> 8) & 0xFF, bg_col & 0xFF, BGP_CRAM, j);
                    writeColorToCRAM((obj0_col >> 16) & 0xFF, (obj0_col >> 8) & 0xFF, obj0_col & 0xFF, OBP_CRAM, j);
                    writeColorToCRAM((obj1_col >> 16) & 0xFF, (obj1_col >> 8) & 0xFF, obj1_col & 0xFF, OBP_CRAM , j + 4);
                }
                return;
            }
        }

    for(int j = 0; j < 4; j++){
        int bg_col = paletteConfigs[0].bg[j];
        int obj0_col = paletteConfigs[0].obj0[j];
        int obj1_col = paletteConfigs[0].obj1[j];

        writeColorToCRAM((bg_col >> 16) & 0xFF, (bg_col >> 8) & 0xFF, bg_col & 0xFF, BGP_CRAM, j);
        writeColorToCRAM((obj0_col >> 16) & 0xFF, (obj0_col >> 8) & 0xFF, obj0_col & 0xFF, OBP_CRAM, j);
        writeColorToCRAM((obj1_col >> 16) & 0xFF, (obj1_col >> 8) & 0xFF, obj1_col & 0xFF, OBP_CRAM, j + 4);
    }
}

void switchCompatibilityMode(){
    if(console_type == DMG_TYPE){
        console_type = DMG_ON_CGB_TYPE;
        hleDmgColorization(ROM);
    } else if(console_type == DMG_ON_CGB_TYPE){
        console_type = DMG_TYPE;
    }

    initColorPalette();
}