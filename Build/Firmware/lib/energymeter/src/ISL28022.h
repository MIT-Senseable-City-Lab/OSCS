#include<Wire.h>

// I2C address of the ISL28022, 0x40(64)
//#define Addr 0x45 //power meter on PV
//#define Addr 0x42 //power meter on battery

#define BATT_SENSING 1
#define PV_SENSING 0

//#define PV_SHUNT_RES_VAL 0.040 		//40 mOhm for whiteburn V1 and V2
#define PV_SHUNT_RES_VAL 0.020 		//40 mOhm for whiteburn V3
#define BAT_SHUNT_RES_VAL 0.0075		// 7.5 mOhm


#define ISL28022_BUS_REG   			0x02
#define ISL28022_CONF_REG			0x00
#define ISL28022_SH_V_REG			0x01
#define ISL28022_BUS_V_REG			0x02
#define ISL28022_POWER_REG			0x03
#define ISL28022_CURR_REG			0x04
#define ISL28022_CALIB_REG			0x05
#define ISL28022_SH_V_TH_REG		0x06
#define ISL28022_BUS_V_TH_REG		0x07
#define ISL28022_DCS_IT_REG			0x08
#define ISL28022_AUX_REG			0x09
  

/* Exported types ------------------------------------------------------------*/
/** @defgroup ISL_Control_Register_Enum
  * @{
  */ 
/** RESET Bit */
typedef enum
{
	ISL_CONF_RST = (uint16_t)(1 << 15)
}ISL_ResetBit_Enum;

/** Bus Voltage Range */
typedef enum
{
  ISL_CONF_BRNG_16	= (uint16_t)(0x00 << 13),		/**< 16V IS 12 BITS WIDE */
  ISL_CONF_BRNG_32	= (uint16_t)(0x01 << 13),		/**< 32V IS 13 BITS WIDE */
  ISL_CONF_BRNG_60	= (uint16_t)(0x02 << 13),		/**< 60V IS 14 BITS WIDE */
}ISL_BusVoltageRange_Enum;

/** Current Reg Bit Handler*/
typedef enum
{
	ISL_CUR_BIT_15 = (uint16_t)(0x01 << 15),
}ISL_CurrentBitHandler_Enum;

/** Shunt Voltage measurement */
typedef enum
{
	ISL_CONF_PG_40	= (uint16_t)(0x00 << 11),			/**< Gain 1 �40mV		(12Bit wide) */
	ISL_CONF_PG_80	= (uint16_t)(0x01 << 11),			/**< Gain 2 �80mV		(13Bit wide) */
	ISL_CONF_PG_160	= (uint16_t)(0x02 << 11),			/**< Gain 4 �160mV	(14Bit wide) */
	ISL_CONF_PG_320	= (uint16_t)(0x03 << 11)			/**< Gain 8 �320mV	(15Bit wide) */
}ISL_ShuntMeasureGain_Enum;

/**	ADC Resolution Accuracy */
typedef enum
{
	ISL_ADC_RES_12B	=	(uint16_t)(0x01 << 12),
	ISL_ADC_RES_13B	=	(uint16_t)(0x01 << 13),
	ISL_ADC_RES_14B	=	(uint16_t)(0x01 << 14),
	ISL_ADC_RES_15B	=	(uint16_t)(0x01 << 15)
}ISL_ADCResolutionAccuracy_Enum;

/** Bus ADC Resolution/Averaging for 15Bit Result */
typedef enum
{
	ISL_CONF_BADC_12B		= (uint16_t)(0x00 << 7),		/**< 12 Bit 72 us */	
	ISL_CONF_BADC_13B		= (uint16_t)(0x01 << 7),		/**< 13 Bit 132 us */	
	ISL_CONF_BADC_14B		= (uint16_t)(0x02 << 7),		/**< 14 Bit 258 us */	
	ISL_CONF_BADC_15B		= (uint16_t)(0x03 << 7),		/**< 15 Bit 508 us */	
	ISL_CONF_BADC_1		= (uint16_t)(0x09 << 7),		  /**< 2 Sample 1.01 ms */
	ISL_CONF_BADC_2		= (uint16_t)(0x0A << 7),		  /**< 4 Sample 2.01 ms */
	ISL_CONF_BADC_4		= (uint16_t)(0x0B << 7),		  /**< 8 Sample 4.01 ms */
	ISL_CONF_BADC_8		= (uint16_t)(0x0C << 7),		  /**< 16 Sample 8.01 ms */
	ISL_CONF_BADC_16	= (uint16_t)(0x0D << 7),		  /**< 32 Sample 16.01 ms */
	ISL_CONF_BADC_32	= (uint16_t)(0x0E << 7),		  /**< 64 Sample 32.01 ms */
	ISL_CONF_BADC_64	= (uint16_t)(0x0F << 7)			  /**< 128 Sample 64.01 ms */
}ISL_BusADCSampleTime_Enum;

/** Shunt ADC Resolution/Averaging for 15Bit result */
typedef enum
{
	ISL_CONF_SADC_12B		= (uint16_t)(0x00 << 3),		/**< 12 Bit 72 us */	
	ISL_CONF_SADC_13B		= (uint16_t)(0x01 << 3),		/**< 13 Bit 132 us */	
	ISL_CONF_SADC_14B		= (uint16_t)(0x02 << 3),		/**< 14 Bit 258 us */	
	ISL_CONF_SADC_15B		= (uint16_t)(0x03 << 3),		/**< 15 Bit 508 us */	
	ISL_CONF_SADC_1		= (uint16_t)(0x09 << 3),		  /**< 2 Sample 1.01 ms */
	ISL_CONF_SADC_2		= (uint16_t)(0x0A << 3),		  /**< 4 Sample 2.01 ms */
	ISL_CONF_SADC_4		= (uint16_t)(0x0B << 3),		  /**< 8 Sample 4.01 ms */
	ISL_CONF_SADC_8		= (uint16_t)(0x0C << 3),		  /**< 16 Sample 8.01 ms */
	ISL_CONF_SADC_16	= (uint16_t)(0x0D << 3),		  /**< 32 Sample 16.01 ms */
	ISL_CONF_SADC_32	= (uint16_t)(0x0E << 3),		  /**< 64 Sample 32.01 ms */
	ISL_CONF_SADC_64	= (uint16_t)(0x0F << 3)			  /**< 128 Sample 64.01 ms */
}ISL_ShuntADCSampleTime_Enum;

/** Shunt FS PGA Range */
typedef enum
{
	ISL_SHUNT_FS_PGA_40		=	(uint16_t)(0x28),		/** 40mV PGA Range*/
	ISL_SHUNT_FS_PGA_80 	=	(uint16_t)(0x50),		/** 80mV PGA Range*/
	ISL_SHUNT_FS_PGA_160 	=	(uint16_t)(0xA0),		/** 160mV PGA Range*/
	ISL_SHUNT_FS_PGA_320	=	(uint16_t)(0x140)		/** 320mV PGA Range*/
}ISL_ShuntFSRange_Enum;
	
/** Operating Mode */
typedef enum
{
	ISL_CONF_MODE_PW_DN		= (uint16_t)0x00,				/**< Power Down */
	ISL_CONF_MODE_SV_TR		= (uint16_t)0x01,				/**< Shunt voltage, triggered */
	ISL_CONF_MODE_BV_TR		= (uint16_t)0x02,				/**< Bus voltage, triggered */
	ISL_CONF_MODE_SB_TR		= (uint16_t)0x03,				/**< Shunt and bus, triggered */
	ISL_CONF_MODE_ADC_OFF	= (uint16_t)0x04,				/**< ADC off (disabled) */
	ISL_CONF_MODE_SV_CON	= (uint16_t)0x05,				/**< Shunt Voltage, continuous */
	ISL_CONF_MODE_BV_CON	= (uint16_t)0x06,				/**< Bus voltage, continuous */
	ISL_CONF_MODE_SB_CON	= (uint16_t)0x07				/**< Shunt and bus, continuous */
}ISL_OperatingMode_Enum;

/** Aux Control Registre Group */
typedef enum
{
	ISL_AUX_EXT_CLK_EN		= (uint16_t)(1 << 6),		/**< External Clock Enable */
	ISL_AUX_IT_EN			= (uint16_t)(1 << 7),		/**< Interrupt Enable */
	ISL_AUX_FORCE_IT_EN		= (uint16_t)(1 << 8)		/**< Force Interrupt Bit */
}ISL_AuxControlReg_Enum;

class ISL28022{
    public:
    ISL28022();
    void begin(uint8_t I2Caddr, bool type);
    float getBusVoltage_V(void);
    float getShuntVoltage_mV(void);
    float getCurrent_mA(void);
	float Calc_ShuntVoltage(uint16_t);

    private:
	uint8_t Addr;
	bool type;
	float current_lsb_res_;
	float vbus_lsb_res_;
	float vshunt_lsb_res;
	uint16_t cal_reg_value_; 

    uint16_t ReadReg(uint8_t islAddr, uint16_t regAddr);
	uint8_t WriteReg(uint8_t islAddr, uint16_t data, uint16_t regAddr);
    float calcCurrent(uint16_t data);

};