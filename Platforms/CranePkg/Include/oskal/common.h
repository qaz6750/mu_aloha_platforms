#pragma once
// Size ops
#define CR_SIZE_1B 1ULL
#define CR_SIZE_1KB (1024ULL * CR_SIZE_1B)
#define CR_SIZE_1MB (1024ULL * CR_SIZE_1KB)
#define CR_SIZE_1GB (1024ULL * CR_SIZE_1MB)
#define CR_SIZE_B(x) ((x) * CR_SIZE_1B)
#define CR_SIZE_KB(x) ((x) * CR_SIZE_1KB)
#define CR_SIZE_MB(x) ((x) * CR_SIZE_1MB)
#define CR_SIZE_GB(x) ((x) * CR_SIZE_1GB)

// Bit ops
#define __ffs(x) __builtin_ffsll(x)
#define BIT(x) (1ULL << (x))
#define GEN_MSK(high, low) (((BIT((high) + 1) - 1) & ~((BIT(low)) - 1)))

#define SET_BITS(val, mask) ((val) | (unsigned long long)(mask))
#define CLR_BITS(val, mask) ((val) & ~(unsigned long long)(mask))

#define SET_FIELD(val, mask)                                                   \
  (((unsigned long long)(val)                                                  \
    << __builtin_ctzll((unsigned long long)(mask))) &                          \
   (unsigned long long)(mask))

#define GET_FIELD(val, mask) (((val) & (mask)) >> (__ffs(mask) - 1))

// ARM GIC related conversion macros
// x means the irq number in device tree.
#define GIC_SGI(x) (x)
#define GIC_PPI(x) (16ULL + (x))
#define GIC_SPI(x) (32ULL + (x))
#define GIC_LPI(x) (8192ULL + (x))