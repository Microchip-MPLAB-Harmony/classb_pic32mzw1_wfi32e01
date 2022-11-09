# CLASSB\_SST\_WDT\_Recovery

**Function**

```c
static void CLASSB_SST_WDT_Recovery(void);
```

**Summary**

This function is called if a WDT reset is caused while a startup self-test is running.

**Description**

This function is used inside the CLASSB\_Init\(\) function. When the device comes back from a WDT reset, if there has been a startup self-test running, it is assumed that the WDT reset has happened because a Class B self-test has taken more time that the WDT timeout period. In this case, the CLASSB\_SST\_WDT\_Recovery\(\) function is called from CLASSB\_Init\(\).

**Precondition**

None.

**Parameters**

None.

**Returns**

None.

**Example**

```c
if ((RCON & (_RCON_CMR_MASK | _RCON_EXTR_MASK |
            _RCON_SWR_MASK | _RCON_DMTO_MASK | _RCON_WDTO_MASK |
            _RCON_BOR_MASK | _RCON_POR_MASK | _RCON_VBPOR_MASK |
            _RCON_VBAT_MASK | _RCON_PORIO_MASK | _RCON_PORCORE_MASK)) == RSTC_RCAUSE_WDT_Msk)
{
    if (*classb_test_in_progress == CLASSB_TEST_STARTED)
    {
        CLASSB_SST_WDT_Recovery();
    }
}
```

**Remarks**

This function is for the internal use of the Class B library.

**Parent topic:**[MPLAB® Harmony Class B Library for PIC32MZ W1 devices](GUID-B046F97C-6BDC-45FC-BC1F-8C54B8F6F09A.md)

