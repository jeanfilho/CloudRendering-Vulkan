#include "stdafx.h"
#include "Validation.h"

void ValidCheck(VkResult result)
{
    if (result == 0) return;
    printf("VkResult %d\n", result);
    if (result < 0)
        abort();
}
