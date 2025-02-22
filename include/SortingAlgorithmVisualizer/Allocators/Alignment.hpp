#pragma once

#include <cstddef>
#include <cstdint>


bool IsValidAlignment( size_t alignment );

void* AlignAddress( void* address, size_t alignment );
uintptr_t AlignAddress( uintptr_t address, size_t alignment );
