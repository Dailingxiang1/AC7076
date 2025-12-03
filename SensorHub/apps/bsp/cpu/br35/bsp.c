#include "includes.h"
#include "bsp.h"
#include "iic_api.h"

void bsp_init()
{
#if P11_HW_IIC_EN
    hw_iic_master_module_init();
#endif//P11_HW_IIC_EN

}
