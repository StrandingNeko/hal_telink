/********************************************************************************************************
 * @file    ext_rf.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd.
 *          All rights reserved.
 *
 *          The information contained herein is confidential property of Telink
 *          Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *          of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *          Co., Ltd. and the licensee or the terms described here-in. This heading
 *          MUST NOT be removed from this file.
 *
 *          Licensee shall not delete, modify or alter (or permit any third party to delete, modify, or
 *          alter) any information contained herein in whole or in part except as expressly authorized
 *          by Telink semiconductor (shanghai) Co., Ltd. Otherwise, licensee shall be solely responsible
 *          for any claim to the extent arising out of or relating to such deletion(s), modification(s)
 *          or alteration(s).
 *
 *          Licensees are granted free, non-transferable use of the information in this
 *          file under Mutual Non-Disclosure Agreement. NO WARRANTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#ifndef DRIVERS_TL721X_EXT_DRIVER_DRIVER_LIB_EXT_RF_H_
#define DRIVERS_TL721X_EXT_DRIVER_DRIVER_LIB_EXT_RF_H_

#include "types.h"

#ifndef RF_ACCESS_CODE_DEFAULT_THRESHOLD
#define RF_ACCESS_CODE_DEFAULT_THRESHOLD    (31)    //0x1e  . BQB may use 32. Coded PHY may use 0xF0
#endif


#ifndef FAST_SETTLE
#define FAST_SETTLE         0
#endif



enum{
    //BLE mode
    FLD_RF_BRX_SN_INIT          =   BIT(4),
    FLD_RF_BRX_NESN_INIT        =   BIT(5),
    FLD_RF_BTX_SN_INIT          =   BIT(6),
    FLD_RF_BTX_NESN_INIT        =   BIT(7),
};

//*********************Note: CRYPT*****************************/
enum{
    FLD_TLK_CRYPT_ENABLE        =   BIT(1),
    FLD_TLK_MST_SLV             =   BIT(2),
};

#define    reg_rf_tlk_sk0       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xa4)
#define    reg_rf_tlk_sk1       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xa8)
#define    reg_rf_tlk_sk2       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xac)
#define    reg_rf_tlk_sk3       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xb0)

#define    reg_tlk_sk(v)        REG_ADDR32(REG_BASEBAND_BASE_ADDR + 0xa4 + (v*4))



#define    reg_rf_tlk_iv0       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xb4)
#define    reg_rf_tlk_iv1       REG_ADDR32(REG_BASEBAND_BASE_ADDR+0xb8)

#define    reg_rf_tlk_aad       REG_ADDR8(REG_BASEBAND_BASE_ADDR+0xbc)

#define    reg_ccm_control              REG_ADDR8(REG_BB_LL_BASE_ADDR+0x3f)


enum{
    FLD_R_TXLEN_FLAG            = BIT(0),
};

//39 bits
#define     reg_rf_tx_ccm_pkt_cnt0_31       REG_ADDR32(REG_BB_LL_BASE_ADDR+0x40)
#define     reg_rf_tx_ccm_pkt_cnt32_38      REG_ADDR8(REG_BB_LL_BASE_ADDR+0x44)

#define     reg_rf_rx_ccm_pkt_cnt0_31       REG_ADDR32(REG_BB_LL_BASE_ADDR+0x48)
#define     reg_rf_rx_ccm_pkt_cnt32_38      REG_ADDR8(REG_BB_LL_BASE_ADDR+0x4c)

//*********************Note: must write and read one word*****************************/
#define REG_CV_LLBT_BASE_ADDR                           (0x160000)

#define reg_cv_intcntl                                  REG_ADDR32(REG_CV_LLBT_BASE_ADDR +0x0c)

#define reg_cv_llbt_rpase_cntl                          REG_ADDR32(REG_CV_LLBT_BASE_ADDR+0x288)

enum
{
    FLD_CV_PRAND            = BIT_RNG(0,23),
    FLD_CV_IRK_NUM          = BIT_RNG(24,27),
    FLD_CV_GEN_RES          = BIT(29),
    FLD_CV_RPASE_START      = BIT(30),
    FLD_CV_PRASE_ENABLE     = BIT(31),
};

#define reg_cv_llbt_hash_status                         REG_ADDR32(REG_CV_LLBT_BASE_ADDR+0x28c)

enum
{
    FLD_CV_hash                 = BIT_RNG(0,23),
    FLD_CV_IRK_CNT              = BIT_RNG(24,27),
    FLD_CV_RPASE_STATUS_CLR     = BIT(28),
    FLD_CV_RPASE_STATUS         = BIT_RNG(29,30),
    FLD_CV_HASH_MATCH           = BIT(31),
};


#define reg_cv_llbt_irk_ptr                 REG_ADDR32(REG_CV_LLBT_BASE_ADDR+0x290)

#define CLEAR_ALL_RFIRQ_STATUS              reg_rf_irq_status = 0xffff;reg_rf_irq_status_h = 0xff; reg_rf_irq_status_h1 = 0x07
#define RF_CLEAR_ALL_IRQ_MASK               reg_rf_irq_mask = 0;reg_rf_irq_mask1=0
#define DMA_RFRX_LEN_HW_INFO                0   // 826x: 8
#define DMA_RFRX_OFFSET_HEADER              4   // 826x: 12
#define DMA_RFRX_OFFSET_RFLEN               5   // 826x: 13
#define DMA_RFRX_OFFSET_DATA                6   // 826x: 14

#define RF_TX_PACKET_DMA_LEN(rf_data_len)       (((rf_data_len)+3)/4)|(((rf_data_len) % 4)<<22)
#define DMA_RFRX_OFFSET_CRC24(p)            (p[DMA_RFRX_OFFSET_RFLEN]+6)  //data len:3
#define DMA_RFRX_OFFSET_TIME_STAMP(p)       (p[DMA_RFRX_OFFSET_RFLEN]+9)  //data len:4
#define DMA_RFRX_OFFSET_FREQ_OFFSET(p)      (p[DMA_RFRX_OFFSET_RFLEN]+13) //data len:2
#define DMA_RFRX_OFFSET_RSSI(p)             (p[DMA_RFRX_OFFSET_RFLEN]+15) //data len:1, signed
#define DMA_RFRX_OFFSET_STATUS(p)           (p[DMA_RFRX_OFFSET_RFLEN]+16)

#define RF_BLE_RF_PAYLOAD_LENGTH_OK(p)                  ( *((unsigned int*)p) == (unsigned int)(p[5]+13))               //dma_len must 4 byte aligned
#define RF_BLE_RF_PACKET_CRC_OK(p)                      ((p[(p[5]+5 + 11)] & 0x01) == 0x0)
#define RF_BLE_PACKET_VALIDITY_CHECK(p)                 (RF_BLE_RF_PAYLOAD_LENGTH_OK(p) && RF_BLE_RF_PACKET_CRC_OK(p))

#define RF_BLE_RF_PACKET_CRC_OK_HW_ECC(p)                       ((p[p[5]+5+11-4] & 0x01) == 0x0)

#define rf_set_tx_packet_address(addr)                  (reg_bb_dma_src_addr(RF_TX_DMA)=(unsigned int)addr)





//RF BLE Minimum TX Power LVL (unit: 1dBm)
extern const char  ble_rf_min_tx_pwr;
//RF BLE Maximum TX Power LVL (unit: 1dBm)
extern const char  ble_rf_max_tx_pwr;
//RF BLE Current TX Path Compensation
extern  signed short ble_rf_tx_path_comp;
//RF BLE Current RX Path Compensation
extern  signed short ble_rf_rx_path_comp;
//Current RF RX DMA buffer point for BLE
extern  unsigned char *ble_curr_rx_dma_buff;


typedef enum {
     RF_ACC_CODE_TRIGGER_AUTO   =    BIT(0),    /**< auto trigger */
     RF_ACC_CODE_TRIGGER_MANU   =    BIT(1),    /**< manual trigger */
} rf_acc_trigger_mode;




extern signed char ble_txPowerLevel;

_attribute_ram_code_ void ble_rf_set_rx_dma(unsigned char *buff, unsigned char size_div_16);

_attribute_ram_code_ void ble_rf_set_tx_dma(unsigned char fifo_dep, unsigned char size_div_16);

_attribute_ram_code_ void ble_rx_dma_config(void);



/**
 * @brief   This function serves to settle adjust for RF Tx.This function for adjust the differ time
 *          when rx_dly enable.
 * @param   txstl_us - adjust TX settle time.
 * @return  none
 */
static _always_inline void  rf_ble_set_tx_settle(unsigned short txstl_us)
{
    REG_ADDR16(0x80170204) = txstl_us;
}

static _always_inline unsigned short rf_read_tx_settle(void)
{
    return REG_ADDR16(0x80170204);
}


/* attention that not bigger than 4095 */
static _always_inline void rf_ble_set_rx_settle( unsigned short rx_stl_us )
{
     write_reg16(0x17020c, rx_stl_us);
}

static _always_inline void  rf_ble_set_rx_wait(unsigned short rx_wait_us)
{
    write_reg8(0x170206, rx_wait_us);
}

static _always_inline void  rf_ble_set_tx_wait(unsigned short tx_wait_us)
{
    write_reg8(0x17020e, tx_wait_us);
}

/**
 * @brief       This function serves to update the value of internal cap.
 * @param[in]   value   - The value of internal cap which you want to set.
 * @return      none.
 */
static _always_inline void  rf_update_internal_capacitance(unsigned char value)
{
    /*
     * afe1v_reg10<5:0>     reg_xo_cdac_ana<5:0>        CDAC value (lowest cap to highest cap)
     * afe1v_reg10<6>       reg_xo_mode_ana             mode control - 0 : AMP_OFF, 1 : AMP_ON.
     *                                                  0 is to support dc coupling and 1 is to support ac coupling
     * afe1v_reg10<7>       reg_xo_cap_off_ana          control of X1 and X2 capacitance values
                                                        0 : cap follows CDAC, 1 : cap OFF
     */
    analog_write_reg8(0x8A, (analog_read_reg8(0x8A) & 0x40)|(value & 0x3f));
}




/**
 * @brief     This function performs to set RF Access Code Threshold.// use for BQB
 * @param[in] threshold   cans be 0-32bits
 * @return    none.
 */
static _always_inline void rf_ble_set_access_code_threshold(u8 threshold)
{
    write_reg8(0x17044e, threshold);
}

/**
 * @brief   This function serves to set RF access code value.
 * @param[in]   ac - the address value.
 * @return  none
 */
static _always_inline void rf_set_ble_access_code_value (unsigned int ac)
{
    write_reg32 (0x80170008, ac);
}

/**
 * @brief   This function serves to set RF access code.
 * @param[in]   p - the address to access.
 * @return  none
 */
static _always_inline void rf_set_ble_access_code (unsigned char *p)
{
    write_reg32 (0x80170008, p[3] | (p[2]<<8) | (p[1]<<16) | (p[0]<<24));
}


/**
 * @brief   This function serves to set RF access code advantage.
 * @param   none.
 * @return  none.
 */
static _always_inline void rf_set_ble_access_code_adv (void)
{
    write_reg32 (0x170008, 0xd6be898e);
}


/**
 * @brief   This function serves to reset function for RF.
 * @param   none
 * @return  none
 *******************need driver change
 */
static _always_inline void reset_sn_nesn(void)
{
    REG_ADDR8(0x80170201) =  0x01;
}




/**
 * @brief   This function serves to trigger accesscode in coded Phy mode.
 * @param   none.
 * @return  none.
 */
static _always_inline void rf_trigger_codedPhy_accesscode(void)
{
    write_reg8(0x170425,read_reg8(0x170425)|0x01);
}

/**
 * @brief     This function performs to enable RF Tx.
 * @param[in] none.
 * @return    none.
 */
static _always_inline void rf_ble_tx_on (void)
{
    write_reg8  (0x80170202, 0x45 | BIT(4));    // TX enable
}

/**
 * @brief     This function performs to done RF Tx.
 * @param[in] none.
 * @return    none.
 */
static _always_inline void rf_ble_tx_done (void)
{
    write_reg8  (0x80170202, 0x45);
}

/**
 * @brief   This function serves to set RX first timeout value.
 * @param[in]   tm - timeout, unit: uS.
 * @return  none.
 */
static _always_inline void rf_set_1st_rx_timeout(unsigned int tm)
{
    reg_rf_ll_rx_fst_timeout = tm;
}


/**
 * @brief   This function serves to set RX timeout value.
 * @param[in]   tm - timeout, unit: uS, range: 0 ~ 0xfff
 * @return  none.
 */
static _always_inline void rf_ble_set_rx_timeout(unsigned short tm)
{
    reg_rf_rx_timeout = tm;
}

/**
 * @brief   This function serve to set the length of preamble for BLE packet.
 * @param[in]   len     -The value of preamble length.Set the register bit<0>~bit<4>.
 * @return      none
 */
static _always_inline void rf_ble_set_preamble_len(unsigned char len)
{
    write_reg8(0x170002,(read_reg8(0x170002)&0xe0)|(len&0x1f));
}

static _always_inline int rf_ble_get_preamble_len(void)
{
    return (read_reg8(0x170002)&0x1f); //[4:0]: BLE preamble length
}



typedef enum{
    FSM_BTX     = 0x81,
    FSM_BRX     = 0x82,
    FSM_PTX     = 0x83,
    FSM_PRX     = 0x84,
    FSM_STX     = 0x85,
    FSM_SRX     = 0x86,
    FSM_TX2RX   = 0x87,
    FSM_RX2TX   = 0x88,
}fsm_mode_e;


/**
 * @brief       This function serves to RF trigger RF state machine.
 * @param[in]   mode  - FSM mode.
 * @param[in]   tx_addr  - DMA TX buffer, if not TX, must be "NULL"
 * @param[in]   tick  - FAM Trigger tick.
 * @return      none.
 */
void rf_start_fsm(fsm_mode_e mode, void* tx_addr, unsigned int tick);

/**
 * @brief      This function serves to reset baseband.this function is same as driver rf_emi_reset_baseband.
 * but rf_emi_reset_baseband is not ram code.
 * @return     none
 */
#define ble_rf_reset_baseband()             rf_clr_dig_logic_state()

/**
 * @brief       This function serves to set RF baseband channel.This function is suitable for ble open PN mode.
 * @param[in]   chn_num  - Bluetooth channel set according to Bluetooth protocol standard.
 * @return      none.
 */
void rf_set_ble_channel (signed char chn_num);

/**
 * @brief     This function performs to switch PHY test mode.
 * @param[in] mode - PHY mode
 * @return    none.
 */
void rf_switchPhyTestMode(rf_mode_e mode);



/*
 * brief:If already know the DMA length value,this API can calculate the real RF length value that is easier for humans to understand.
 * param: dma_len -- the value calculated by this macro "rf_tx_packet_dma_len"
 * return: 0xFFFFFFFE --- error;
 *         other value--- success;
 */
u32 rf_cal_rfLenFromDmaLen(u32 dma_len);


//TODO: merge into driver
enum{
    FLD_RF_SN                     = BIT(0),
};



/**
 * @brief    This function serves to enable zb_rt interrupt source.
 * @return  none
 */
static inline void zb_rt_irq_enable(void)
{
    plic_interrupt_enable(IRQ_ZB_RT);
}



#define RF_RX_WAIT_MIN_VALUE            (0)
#define RF_TX_WAIT_MIN_VALUE            (0)


/*
 * SiHui & QingHua & SunWei sync with Xuqiang.Zhang & Zhiwei.Wang & Kaixin.Chen & Shujuan.chu
 * B91/B92
 * TX settle recommend value by digital team: 108.5uS without fast settle;   108.5-58.5=50 with fast settle
 * we BLE use 110 without fast settle; 110-57=53 with fast settle, here 53 = real settle 45uS + extra 1 preamble 8uS(1M for example)
 *
 * RX settle recommend value by digital team: 108.5uS without fast settle;   85-40=45 with fast settle
 */

#if (FAST_SETTLE)
    #define RX_SETTLE_US                    45
#else
    #define RX_SETTLE_US                    85
#endif


#define RXSET_OPTM_ANTI_INTRF               100  //RX settle value for optimize anti-interference


#define PRMBL_LENGTH_1M                     3   //preamble length for 1M PHY
#define PRMBL_LENGTH_2M                     4   //preamble length for 2M PHY
#define PRMBL_LENGTH_Coded                  11  //preamble length for Coded PHY, never change this value !!!

#define PRMBL_EXTRA_1M                      (PRMBL_LENGTH_1M - 1)   // 1 byte for 1M PHY
#define PRMBL_EXTRA_2M                      (PRMBL_LENGTH_2M - 2)   // 2 byte for 2M
#define PRMBL_EXTRA_Coded                   (PRMBL_LENGTH_Coded -10)                        // 10byte for Coded, 80uS, no extra byte



#if RF_RX_SHORT_MODE_EN//open rx dly

    #if (FAST_SETTLE)
        #define         TX_STL_LEGADV_SCANRSP_REAL                  53  //can change, consider TX packet quality
    #else
        #define         TX_STL_LEGADV_SCANRSP_REAL                  111 //can change, consider TX packet quality
    #endif
    #define         TX_STL_LEGADV_SCANRSP_SET                       (TX_STL_LEGADV_SCANRSP_REAL - PRMBL_EXTRA_1M * 8)  //can not change !!!


    #if (FAST_SETTLE)
        #define         TX_STL_TIFS_REAL_COMMON                     53  //can change, consider TX packet quality
    #else
        #define         TX_STL_TIFS_REAL_COMMON                     114 //can change, consider TX packet quality
    #endif

    #define         TX_STL_TIFS_REAL_1M                             TX_STL_TIFS_REAL_COMMON  //can not change !!!
    #define         TX_STL_TIFS_SET_1M                              (TX_STL_TIFS_REAL_1M - PRMBL_EXTRA_1M * 8)  //can not change !!!

    #define         TX_STL_TIFS_REAL_2M                             TX_STL_TIFS_REAL_COMMON  //can not change !!!
    #define         TX_STL_TIFS_SET_2M                              (TX_STL_TIFS_REAL_2M - PRMBL_EXTRA_2M * 4)  //can not change !!!

    #define         TX_STL_TIFS_REAL_CODED                          TX_STL_TIFS_REAL_COMMON  //can not change !!!
    #define         TX_STL_TIFS_SET_CODED                           (TX_STL_TIFS_REAL_CODED -PRMBL_EXTRA_Coded * 8)      //can not change !!!


    #if (FAST_SETTLE)
        #define         TX_STL_ADV_REAL_COMMON                      53  //can change, consider TX packet quality
    #else
        #define         TX_STL_ADV_REAL_COMMON                      114 //can change, consider TX packet quality
    #endif
    #define         TX_STL_ADV_REAL_1M                              TX_STL_ADV_REAL_COMMON
    #define         TX_STL_ADV_SET_1M                               (TX_STL_ADV_REAL_1M - PRMBL_EXTRA_1M * 8)  //can not change !!!

    #define         TX_STL_ADV_REAL_2M                              TX_STL_ADV_REAL_COMMON
    #define         TX_STL_ADV_SET_2M                               (TX_STL_ADV_REAL_2M - PRMBL_EXTRA_2M * 4)  //can not change !!!

    #define         TX_STL_ADV_REAL_CODED                           TX_STL_ADV_REAL_COMMON
    #define         TX_STL_ADV_SET_CODED                            TX_STL_ADV_REAL_CODED  //can not change !!!


    #define         TX_STL_AUTO_MODE_1M                             (127 - PRMBL_EXTRA_1M * 8)
    #define         TX_STL_AUTO_MODE_2M                             (133 - PRMBL_EXTRA_2M * 4)
    #define         TX_STL_AUTO_MODE_CODED                          (127 - PRMBL_EXTRA_Coded * 8)


    #if (FAST_SETTLE)
        #define     TX_STL_BTX_1ST_PKT_REAL                         53 //110 - 57 = 53
    #else
        /* normal mode(no fast settle): for ACL Central, tx settle real 110uS or 107uS or even 105uS, not big difference,
         * but for CIS Central, timing is very urgent considering T_MSS between two sub_event, so SiHui use 107, we keep this set
         * fast settle mode:  */
        #define     TX_STL_BTX_1ST_PKT_REAL                         (110 - 3) //3 is total switch delay time
    #endif

    #define         TX_STL_BTX_1ST_PKT_SET_1M                       (TX_STL_BTX_1ST_PKT_REAL - PRMBL_EXTRA_1M * 8)
    #define         TX_STL_BTX_1ST_PKT_SET_2M                       (TX_STL_BTX_1ST_PKT_REAL - PRMBL_EXTRA_2M * 4)
    #define         TX_STL_BTX_1ST_PKT_SET_CODED                    TX_STL_BTX_1ST_PKT_REAL
#else
    #error "add code here, TX settle time"
#endif


/* AD convert delay : timing cost on RF analog to digital convert signal process:
 *                  Eagle   1M: 20uS       2M: 10uS;      500K(S2): 14uS    125K(S8):  14uS
 *                  Jaguar  1M: 20uS       2M: 10uS;      500K(S2): 14uS    125K(S8):  14uS
 */
#define AD_CONVERT_DLY_1M                                           19  //before:20. Jaguar T_IFS need 32M + AD_Convert=19, tested by kai.jia at 2022-11-17
#define AD_CONVERT_DLY_2M                                           10
#define AD_CONVERT_DLY_CODED                                        14

#define OTHER_SWITCH_DELAY_1M                                       0
#define OTHER_SWITCH_DELAY_2M                                       3
#define OTHER_SWITCH_DELAY_CODED                                    3


#define HW_DELAY_1M                                                 (AD_CONVERT_DLY_1M + OTHER_SWITCH_DELAY_1M)
#define HW_DELAY_2M                                                 (AD_CONVERT_DLY_2M + OTHER_SWITCH_DELAY_2M)
#define HW_DELAY_CODED                                              (AD_CONVERT_DLY_CODED + OTHER_SWITCH_DELAY_CODED)

static inline void rf_ble_set_1m_phy(void)
{
    //aura_1m
    write_reg8(0x17063d,0x61);//ble:bw_code.
    write_reg8(0x170620,0x10);//sc_code.
    write_reg8(0x170621,0x0a);//if_freq,IF = 1Mhz,BW = 1Mhz.
    write_reg8(0x170622,0x20);//RADIO BLE_MODE_TX,1MBPS:bit<0>;VCO_TRIM_KV:bit<1-3>;HPMC_EXP_DIFF_COUNT_L:bit<4-7>.
    write_reg8(0x170623,0x23);//HPMC_EXP_DIFF_COUNT_H.
    write_reg8(0x170422,0x00);//modem:BLE_MODE_TX,1MBPS.
    write_reg8(0x17044e,RF_ACCESS_CODE_DEFAULT_THRESHOLD);//ble sync threshold:To modem.

    write_reg8(0x17063f,0x00);//250k modulation index:telink add rx for 250k/500k.

    //rx_cont_mode
    write_reg8(0x170420,0xc8);// script cc. rx continue mode on:bit<3>

    write_reg8(0x17044d,0x01);//r_rxchn_en_i:To modem.
    write_reg8(0x170421,0x00);//modem:ZIGBEE_MODE:01
    write_reg8(0x170423,0x00);//modem:ZIGBEE_MODE_TX.
    write_reg8(0x170426,0x00);//modem:sync rst sel,for zigbee access code sync.
    write_reg8(0x17042a,0x10);//modem:disable MSK.
    write_reg8(0x17043d,0x00);//modem:zb_sfd_frm_ll.
    write_reg8(0x17042c,0x38);//modem:zb_dis_rst_pdet_isfd.
    write_reg8(0x170436,0xb7);//LR_NUM_GEAR_L.
    write_reg8(0x170437,0x0e);//LR_NUM_GEAR_H.
    write_reg8(0x170438,0xb6);//LR_TIM_EDGE_DEV.0xc4->0xb6
    write_reg8(0x170439,0x71);//LR_TIM_REC_CFG_1.
    write_reg8(0x170473,0x01);//TOT_DEV_RST.
    write_reg8(0x17049a,0x00);//tx_tp_align.

    //agc_table_1m
    write_reg8(0x1704c2,0x3b);//grx_0.
    write_reg8(0x1704c3,0x46);//grx_1.
    write_reg8(0x1704c4,0x54);//grx_2.
    write_reg8(0x1704c5,0x5f);//grx_3.
    write_reg8(0x1704c6,0x6c);//grx_4.
    write_reg8(0x1704c7,0x7a);//grx_5.
    write_reg8(0x1704c8,0x39);//default:0x00->0x39 Gain offset to compensate system error

    //ble1m_setup
    write_reg32(0x170000,0x5440080f|PRMBL_LENGTH_1M<<16);//tx_mode.
//  write_reg8(0x170001,0x08);//PN.
//  write_reg8(0x170002,0x40| PRMBL_LENGTH_1M<<16);//preamble length
//  write_reg8(0x170003,0x54);//bit<0:1>private mode control.
    write_reg8(0x170004,0xf1);//bit<4>mode:1->1m;bit<0:3>:ble head.
    write_reg8(0x170005,0x04);//lr mode bit<4:5>

    write_reg8(0x170021,0xa1);//rx packet len 0 enable.
                              //bit<5>:write packet length filed into sram
    write_reg8(0x170022,0x00);//rxchn_man_en.

    write_reg8(0x17044c,0x0c);//RX:acc_len modem.0x4c->0x0c



    write_reg8(0x1704bb,0x00);//disable 2 stage filter
    write_reg8(0x17043e,0x81);//BIT<7>:0 new ,1 old;pm2fm suppress more than pi/4
    //The following register configurations are configured in zigbee/hybee mode, which maintains register defaults
    write_reg8(0x170014,0x7a);//access code for hybee 500K.
    write_reg8(0x170015,0x35);//access code for hybee 500K.
    write_reg8(0x170132,0x01);//zigbee PHR field enable 1: phr field length embedded in data stream; 0: phr field length from reg ctrl as like private SB packet
    write_reg8(0x17043b,0x1c);//Rx: sfd match symb0 num
    write_reg8(0x170450,0x3f);//dciq edr  auto
    write_reg8(0x170451,0x0e);//edr dcoc auto
    //pdet sync thd default:0x190 [9'd400]
    write_reg8(0x1704e0,0x90);//sync_thd
    write_reg8(0x1704e1,0x19);//sync_thd    0x1f4 == 500  0x1c2 == 450  0x1a4 == 420
    write_reg8(0x1704e2,0x18);//pdet_hardec_thd
    write_reg8(0x17062d,0x00);//CHNL_NUM switch by sw en
    write_reg8(0x170799,0x00);//ZB_FREQ_FIXED_OW

    //The following registers are configured in BLE 125K and BLE 500K mode, which maintains the register defaults
    write_reg8(0x1704f0,0x1c);//defaults 0x1c. lr_s8_pdet synv_success threshold 0~32
}


static inline void rf_ble_set_2m_phy(void)
{
//  rf_set_ble_2M_mode();
    //aura_2m
    write_reg8(0x17063d,0x41);//ble:bw_code.
    write_reg8(0x170620,0x00);//sc_code.
    write_reg8(0x170621,0x2a);//if_freq,IF = 1Mhz,BW = 1Mhz.
    write_reg8(0x170622,0x43);//HPMC_EXP_DIFF_COUNT_L.
    write_reg8(0x170623,0x26);//HPMC_EXP_DIFF_COUNT_H.
    write_reg8(0x170422,0x01);//modem:BLE_MODE_TX,2MBPS.
    write_reg8(0x17044e,RF_ACCESS_CODE_DEFAULT_THRESHOLD);//ble sync threshold:To modem.0x20->0x1e


    write_reg8(0x17063f,0x00);//250k modulation index:telink add rx for 250k/500k.

    write_reg8(0x1704bb,0x20);//2 stage filter,

    //rx_cont_mode
    write_reg8(0x170420,0xc8);

    write_reg8(0x17044d,0x01);//r_rxchn_en_i:To modem.
    write_reg8(0x170421,0x00);//modem:ZIGBEE_MODE:01.
    write_reg8(0x170423,0x00);//modem:ZIGBEE_MODE_TX.
    write_reg8(0x170426,0x00);//modem:sync rst sel,for zigbee access code sync.
    write_reg8(0x17042a,0x10);//modem:disable MSK.
    write_reg8(0x17043d,0x00);//modem:zb_sfd_frm_ll.
    write_reg8(0x17042c,0x38);//modem:zb_dis_rst_pdet_isfd.
    write_reg8(0x170436,0xb7);//LR_NUM_GEAR_L.
    write_reg8(0x170437,0x0e);//LR_NUM_GEAR_H.
    write_reg8(0x170438,0xb6);//LR_TIM_EDGE_DEV.0xc4->0xb6
    write_reg8(0x170439,0x71);//LR_TIM_REC_CFG_1.
    write_reg8(0x170473,0x01);//TOT_DEV_RST.
    write_reg8(0x17049a,0x00);//tx_tp_align.

    //agc_table
    write_reg8(0x1704c2,0x3f);//grx_0.
    write_reg8(0x1704c3,0x48);//grx_1.
    write_reg8(0x1704c4,0x56);//grx_2.
    write_reg8(0x1704c5,0x61);//grx_3.
    write_reg8(0x1704c6,0x6c);//grx_4.
    write_reg8(0x1704c7,0x7b);//grx_5.
    write_reg8(0x1704c8,0x39);//default:0x00->0x39 Gain offset to compensate system error

    write_reg32(0x170000,0x5440080f|PRMBL_LENGTH_2M<<16);//tx_mode.
//  write_reg8(0x170001,0x08);//PN.
//  write_reg8(0x170002,0x43);//preamble len.
//  write_reg8(0x170003,0x54);//bit<0:1>private mode control.
    write_reg8(0x170004,0xe1);//bit<4>mode:1->1m;bit<0:3>:ble head.
    write_reg8(0x170005,0x04);//lr mode bit<4:5>

    write_reg8(0x170021,0xa1);//rx packet len 0 enable.
    write_reg8(0x170022,0x00);//rxchn_man_en.

    write_reg8(0x17044c,0x0c);//RX:acc_len modem.


    write_reg8(0x17043e,0x81);//BIT<7>:0 new ,1 old;pm2fm suppress more than pi/4
    //The following register configurations are configured in zigbee/hybee mode, which maintains register defaults
    write_reg8(0x170014,0x7a);//access code for hybee 500K.
    write_reg8(0x170015,0x35);//access code for hybee 500K.
    write_reg8(0x170132,0x01);//zigbee PHR field enable 1: phr field length embedded in data stream; 0: phr field length from reg ctrl as like private SB packet
    write_reg8(0x17043b,0x1c);//Rx: sfd match symb0 num
    write_reg8(0x170450,0x3f);//dciq edr  auto
    write_reg8(0x170451,0x0e);//edr dcoc auto
    //pdet sync thd default:0x190 [9'd400]
    write_reg8(0x1704e0,0x90);//sync_thd
    write_reg8(0x1704e1,0x19);//sync_thd    0x1f4 == 500  0x1c2 == 450  0x1a4 == 420
    write_reg8(0x1704e2,0x18);//pdet_hardec_thd
    write_reg8(0x17062d,0x00);//CHNL_NUM switch by sw en
    write_reg8(0x170799,0x00);//ZB_FREQ_FIXED_OW


    //The following registers are configured in BLE 125K and BLE 500K mode, which maintains the register defaults
    write_reg8(0x1704f0,0x1c);//defaults 0x1c. lr_s8_pdet synv_success threshold 0~32
}




static inline void rf_ble_set_coded_phy_common(void)
{
       write_reg8(0x17063d,0x61);//ble:bw_code.
       write_reg8(0x170620,0x10);//sc_code.
       write_reg8(0x170621,0x0a);//if_freq,IF = 1Mhz,BW = 1Mhz.
       write_reg8(0x170622,0x20);//HPMC_EXP_DIFF_COUNT_L.
       write_reg8(0x170623,0x23);//HPMC_EXP_DIFF_COUNT_H.
       write_reg8(0x170422,0x00);//modem:BLE_MODE_TX,2MBPS.
       write_reg8(0x17044e,0xf0);//ble sync threshold:To modem.


       write_reg8(0x17063f,0x00);//250k modulation index:telink add rx for 250k/500k.

       write_reg8(0x170473,0xa1);//TOT_DEV_RST.
       //write_reg8(0x170436,0xee);//LR_NUM_GEAR_L.  (move to rf_ble_set_coded_phy_s2 & rf_ble_set_coded_phy_s8)
       write_reg8(0x170437,0x0c);//LR_NUM_GEAR_H.
       write_reg8(0x170438,0xb8);//LR_TIM_EDGE_DEV.
       write_reg8(0x170439,0x7d);//LR_TIM_REC_CFG_1.

       write_reg8(0x170420,0xc9);// script cc.

       write_reg8(0x17044d,0x01);//r_rxchn_en_i:To modem.
       //write_reg8(0x170421,0x00);//modem:ZIGBEE_MODE:01.  (move to rf_ble_set_coded_phy_s2 & rf_ble_set_coded_phy_s8)
       write_reg8(0x170423,0x00);//modem:ZIGBEE_MODE_TX.
       write_reg8(0x170426,0x00);//modem:sync rst sel,for zigbee access code sync.
       write_reg8(0x17042a,0x10);//modem:disable MSK.
       write_reg8(0x17043d,0x00);//modem:zb_sfd_frm_ll.
       write_reg8(0x17042c,0x38);//modem:zb_dis_rst_pdet_isfd.
       write_reg8(0x17049a,0x00);//tx_tp_align.

       write_reg8(0x1704c2,0x3b);//grx_0.
       write_reg8(0x1704c3,0x46);//grx_1.
       write_reg8(0x1704c4,0x54);//grx_2.
       write_reg8(0x1704c5,0x5f);//grx_3.
       write_reg8(0x1704c6,0x6c);//grx_4.
       write_reg8(0x1704c7,0x7a);//grx_5.
       write_reg8(0x1704c8,0x39);  // Gain offset to compensate system error

       write_reg32(0x170000,0x5440080f|PRMBL_LENGTH_Coded<<16);//tx_mode.
       //write_reg8(0x170000,0x0f);//tx_mode.
       //write_reg8(0x170001,0x08);//PN.
       /*
       *       bit                 default value                       note
       * ---------------------------------------------------------------------------
       * <4: 0>:preamble length        default:0x0a,->0x0b(10byte->11byte) Add 1Byte preamble length to fix Freq Drift Rate marginal fail.
       * modified by zhiwei.wang,confirmed by wenfeng.lou 20240606.jira:http://192.168.48.49:8080/browse/TER-64
       */
       //write_reg8(0x170002,0x4b);//preamble len.
       //write_reg8(0x170003,0x54);//bit<0:1>private mode control.
       write_reg8(0x170004,0xf1);//bit<4>mode:1->1m;bit<0:3>:ble head.
       //write_reg8(0x170005,0x24);//lr mode bit<4:5>  (move to rf_ble_set_coded_phy_s2 & rf_ble_set_coded_phy_s8)

       write_reg8(0x170021,0xa1);//rx packet len 0 enable.


       write_reg8(0x170022,0x00);//rxchn_man_en.
       write_reg8(0x17044c,0x0c);//RX:acc_len modem.

       write_reg8(0x1704bb,0x00);//disable 2 stage filter
       write_reg8(0x17043e,0x81);//BIT<7>:0 new ,1 old;pm2fm suppress more than pi/4
       //The following register configurations are configured in zigbee/hybee mode, which maintains register defaults
       write_reg8(0x170014,0x7a);//access code for hybee 500K.
       write_reg8(0x170015,0x35);//access code for hybee 500K.
       write_reg8(0x170132,0x01);//zigbee PHR field enable 1: phr field length embedded in data stream; 0: phr field length from reg ctrl as like private SB packet
       write_reg8(0x17043b,0x1c);//Rx: sfd match symb0 num
       write_reg8(0x170450,0x3f);//dciq edr  auto
       write_reg8(0x170451,0x0e);//edr dcoc auto
       //pdet sync thd default:0x190 [9'd400]
       write_reg8(0x1704e0,0x90);//sync_thd
       write_reg8(0x1704e1,0x19);//sync_thd    0x1f4 == 500  0x1c2 == 450  0x1a4 == 420
       write_reg8(0x1704e2,0x18);//pdet_hardec_thd
       write_reg8(0x17062d,0x00);//CHNL_NUM switch by sw en
       write_reg8(0x170799,0x00);//ZB_FREQ_FIXED_OW

       write_reg8(0x1704f0,0x1e);//defaults 0x1c->0x1e. lr_s8_pdet synv_success threshold 0~32
}



static inline void rf_ble_set_coded_phy_s2(void)
{
    write_reg8(0x170436,0xee);//LR_NUM_GEAR_L.
    write_reg8(0x170421,0x00);//modem:bit<2> LR_MODEM_SEL,bit<3> LR_VITERBI_SEL
    write_reg8(0x170005,0x24);//lr mode bit<4:5>
}

static inline void rf_ble_set_coded_phy_s8(void)
{
    write_reg8(0x170436,0xf6);//LR_NUM_GEAR_L.
    write_reg8(0x170421,0x00);//modem:ZIGBEE_MODE:01.,bit(7) has no influence, need to check in next version by lihaojie 2024.09.13
    write_reg8(0x170005,0x34);//lr mode bit<4:5>
}


//This is to be compatible in older versions. If you don't use them, you can delete them.
#define rf_trigle_codedPhy_accesscode rf_trigger_codedPhy_accesscode


#if FAST_SETTLE
    typedef struct __attribute__((packed))
    {
        u8 LDO_CAL_TRIM;    //0xea[5:0]
        u8 LDO_RXTXHF_TRIM; //0xee[5:0]
        u8 LDO_RXTXLF_TRIM; //0xee[7:6]  0xed[3:0]
        u8 LDO_PLL_TRIM;    //0xee[5:0]
        u8 LDO_VCO_TRIM;    //0xee[7:6]  0xef[3:0]
        u8 rsvd;
    }Ldo_Trim;

    typedef struct __attribute__((packed))
    {
        unsigned char tx_fast_en;
        unsigned char rx_fast_en;
        unsigned short cal_tbl[40];
        rf_ldo_trim_t   ldo_trim;
        rf_dcoc_cal_t   dcoc_cal;
    }Fast_Settle;
    extern Fast_Settle fast_settle;

    void ble_rf_tx_fast_settle(void);
    void ble_rf_rx_fast_settle(void);
    unsigned short get_rf_hpmc_cal_val(void);
    void set_rf_hpmc_cal_val(unsigned short value);
    unsigned char ble_is_rf_tx_fast_settle_en(void);
    unsigned char ble_is_rf_rx_fast_settle_en(void);
    void get_ldo_trim_val(u8* p);
    void set_ldo_trim_val(u8* p);
    void set_ldo_trim_on(void);

    /**
     *  @brief      this function serve to enable the tx timing sequence adjusted.
     *  @param[in]  none
     *  @return     none
    */
    void ble_rf_tx_fast_settle_en(void);

    /**
     *  @brief      this function serve to disable the tx timing sequence adjusted.
     *  @param[in]  none
     *  @return     none
    */
    void ble_rf_tx_fast_settle_dis(void);


    /**
     *  @brief      this function serve to enable the rx timing sequence adjusted.
     *  @param[in]  none
     *  @return     none
    */
    void ble_rf_rx_fast_settle_en(void);


    /**
     *  @brief      this function serve to disable the rx timing sequence adjusted.
     *  @param[in]  none
     *  @return     none
    */
    void ble_rf_rx_fast_settle_dis(void);



#endif




typedef struct {
    unsigned char  rfMode_init_flag;
    unsigned char  txPower_index;
}ext_rf_t;

extern ext_rf_t blt_extRF;

unsigned char rf_ble_get_tx_pwr_idx( char rfTxPower);

char rf_ble_get_tx_pwr_level(rf_power_level_index_e rfPwrLvlIdx);

#endif /* DRIVERS_TL721X_EXT_DRIVER_DRIVER_LIB_EXT_RF_H_ */
