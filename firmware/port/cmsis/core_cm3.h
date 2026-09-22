#ifndef __CORE_CM3_H_GENERIC_LINUX_PORT
#define __CORE_CM3_H_GENERIC_LINUX_PORT

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __I  volatile const
#define __O  volatile
#define __IO volatile

#define __IM volatile const
#define __OM volatile
#define __IOM volatile

#define __STATIC_INLINE static inline
#define __ASM __asm

__STATIC_INLINE void __NOP(void) { __ASM volatile ("nop"); }
__STATIC_INLINE void __WFI(void) { __ASM volatile ("wfi"); }
__STATIC_INLINE void __WFE(void) { __ASM volatile ("wfe"); }
__STATIC_INLINE void __DSB(void) { __ASM volatile ("dsb 0xF":: :"memory"); }
__STATIC_INLINE void __ISB(void) { __ASM volatile ("isb 0xF":: :"memory"); }
__STATIC_INLINE void __enable_irq(void) { __ASM volatile ("cpsie i" : : : "memory"); }
__STATIC_INLINE void __disable_irq(void) { __ASM volatile ("cpsid i" : : : "memory"); }

typedef struct
{
  __IO uint32_t CTRL;
  __IO uint32_t LOAD;
  __IO uint32_t VAL;
  __I  uint32_t CALIB;
} SysTick_Type;

typedef struct
{
  __I  uint32_t CPUID;
  __IO uint32_t ICSR;
  __IO uint32_t VTOR;
  __IO uint32_t AIRCR;
  __IO uint32_t SCR;
  __IO uint32_t CCR;
  __IO uint8_t  SHP[12];
  __IO uint32_t SHCSR;
  __IO uint32_t CFSR;
  __IO uint32_t HFSR;
  __IO uint32_t DFSR;
  __IO uint32_t MMFAR;
  __IO uint32_t BFAR;
  __IO uint32_t AFSR;
  __I  uint32_t PFR[2];
  __I  uint32_t DFR;
  __I  uint32_t ADR;
  __I  uint32_t MMFR[4];
  __I  uint32_t ISAR[5];
} SCB_Type;

typedef struct
{
  __IO uint32_t ISER[8];
       uint32_t RESERVED0[24];
  __IO uint32_t ICER[8];
       uint32_t RSERVED1[24];
  __IO uint32_t ISPR[8];
       uint32_t RESERVED2[24];
  __IO uint32_t ICPR[8];
       uint32_t RESERVED3[24];
  __IO uint32_t IABR[8];
       uint32_t RESERVED4[56];
  __IO uint8_t  IP[240];
       uint32_t RESERVED5[644];
  __O  uint32_t STIR;
} NVIC_Type;

#define SCS_BASE        (0xE000E000UL)
#define SysTick_BASE    (SCS_BASE + 0x0010UL)
#define NVIC_BASE       (SCS_BASE + 0x0100UL)
#define SCB_BASE        (SCS_BASE + 0x0D00UL)

#define SysTick         ((SysTick_Type *) SysTick_BASE)
#define NVIC            ((NVIC_Type *) NVIC_BASE)
#define SCB             ((SCB_Type *) SCB_BASE)

#define SysTick_CTRL_ENABLE_Pos      0U
#define SysTick_CTRL_ENABLE_Msk      (1UL << SysTick_CTRL_ENABLE_Pos)
#define SysTick_CTRL_TICKINT_Pos     1U
#define SysTick_CTRL_TICKINT_Msk     (1UL << SysTick_CTRL_TICKINT_Pos)
#define SysTick_CTRL_CLKSOURCE_Pos   2U
#define SysTick_CTRL_CLKSOURCE_Msk   (1UL << SysTick_CTRL_CLKSOURCE_Pos)
#define SysTick_CTRL_COUNTFLAG_Pos   16U
#define SysTick_CTRL_COUNTFLAG_Msk   (1UL << SysTick_CTRL_COUNTFLAG_Pos)

#define SysTick_LOAD_RELOAD_Msk      (0xFFFFFFUL)

#define SCB_AIRCR_VECTKEY_Pos       16U
#define SCB_AIRCR_VECTKEY_Msk       (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)
#define SCB_AIRCR_PRIGROUP_Pos      8U
#define SCB_AIRCR_PRIGROUP_Msk      (7UL << SCB_AIRCR_PRIGROUP_Pos)
#define SCB_AIRCR_SYSRESETREQ_Pos   2U
#define SCB_AIRCR_SYSRESETREQ_Msk   (1UL << SCB_AIRCR_SYSRESETREQ_Pos)
#define SCB_SCR_SLEEPDEEP_Pos       2U
#define SCB_SCR_SLEEPDEEP_Msk       (1UL << SCB_SCR_SLEEPDEEP_Pos)

#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS 4U
#endif

__STATIC_INLINE uint32_t SysTick_Config(uint32_t ticks)
{
  if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk) {
    return 1UL;
  }
  SysTick->LOAD = ticks - 1UL;
  SysTick->VAL = 0UL;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  return 0UL;
}

__STATIC_INLINE void NVIC_EnableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)IRQn >= 0) {
    NVIC->ISER[((uint32_t)IRQn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
  }
}

__STATIC_INLINE void NVIC_DisableIRQ(IRQn_Type IRQn)
{
  if ((int32_t)IRQn >= 0) {
    NVIC->ICER[((uint32_t)IRQn) >> 5UL] = (uint32_t)(1UL << (((uint32_t)IRQn) & 0x1FUL));
  }
}

__STATIC_INLINE void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if ((int32_t)IRQn < 0) {
    SCB->SHP[(((uint32_t)IRQn) & 0xFUL) - 4UL] = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & 0xFFUL);
  } else {
    NVIC->IP[(uint32_t)IRQn] = (uint8_t)((priority << (8U - __NVIC_PRIO_BITS)) & 0xFFUL);
  }
}

__STATIC_INLINE uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{
  if ((int32_t)IRQn < 0) {
    return (uint32_t)(SCB->SHP[(((uint32_t)IRQn) & 0xFUL) - 4UL] >> (8U - __NVIC_PRIO_BITS));
  }
  return (uint32_t)(NVIC->IP[(uint32_t)IRQn] >> (8U - __NVIC_PRIO_BITS));
}

__STATIC_INLINE void NVIC_SystemReset(void)
{
  __DSB();
  SCB->AIRCR = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
  __DSB();
  while (1) { __NOP(); }
}

#ifdef __cplusplus
}
#endif

#endif
