#include "common.h"
#include "iap.h"


int main(void)
{
	IAP_USART_Init();
	while(1)
	{
		switch(IAP_FLASH_ReadFlag())
		{
			case APPRUN_FLAG_DATA://jump to app
				if( IAP_RunApp())
					IAP_FLASH_WriteFlag(INIT_FLAG_DATA);
				break;
			case INIT_FLAG_DATA://initialze state (blank mcu)
				IAP_Main_Menu();
				break;
			case UPDATE_FLAG_DATA:// download app state				
				if( !IAP_Update()) 
					IAP_FLASH_WriteFlag(APPRUN_FLAG_DATA);
				else
					IAP_FLASH_WriteFlag(INIT_FLAG_DATA);
				break;
			case UPLOAD_FLAG_DATA:// upload app state
//				if( !IAP_Upload())
//					IAP_FLASH_WriteFlag(APPRUN_FLAG_DATA);
//				else 
//					IAP_FLASH_WriteFlag(INIT_FLAG_DATA);
				break;
			case ERASE_FLAG_DATA:// erase app state
				IAP_Erase();
				IAP_FLASH_WriteFlag(INIT_FLAG_DATA);
				break;
			default:
				break;
		}
	}
}


