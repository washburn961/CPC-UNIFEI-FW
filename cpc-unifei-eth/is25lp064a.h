#pragma once

#define SET_BIT(REG, BIT)    ((REG) |= (BIT))
#define RESET_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define CHECK_BIT(REG, BIT)  ((REG) & (BIT))
#define CHECK_BIT_CLR(REG, BIT)  (!((REG) & (BIT)))

// Status Register Bit Definitions
#define SR_WIP        (1 << 0)  // Write In Progress (WIP) bit
#define SR_WEL        (1 << 1)  // Write Enable Latch (WEL) bit
#define SR_BP0        (1 << 2)  // Block Protection Bit 0 (BP0)
#define SR_BP1        (1 << 3)  // Block Protection Bit 1 (BP1)
#define SR_BP2        (1 << 4)  // Block Protection Bit 2 (BP2)
#define SR_BP3        (1 << 5)  // Block Protection Bit 3 (BP3)
#define SR_QE         (1 << 6)  // Quad Enable (QE) bit
#define SR_SWRD       (1 << 7)  // Status Register Write Disable (SRWD) bit

