#pragma once

#include <cstddef>


class ISorter;
class IAllocator;
class ArenaAllocator;

class Backend;
class Frontend;

const static size_t CacheLineSize {64};
