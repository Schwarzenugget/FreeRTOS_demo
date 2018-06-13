/*****************************************************************************
 *
 * \file
 *
 * \brief Basic WEB Server for AVR32 UC3.
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *****************************************************************************/


/*
  Implements a simplistic WEB server.  Every time a connection is made and
  data is received a dynamic page that shows the current FreeRTOS.org kernel
  statistics is generated and returned.  The connection is then closed.

  This file was adapted from a FreeRTOS lwIP slip demo supplied by a third
  party.
*/

#if (HTTP_USED == 1)


/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include "conf_eth.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "partest.h"

/* Demo includes. */
/* Demo app includes. */
#include "portmacro.h"

/* lwIP includes. */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
#include "netif/loopif.h"
#endif

/* ethernet includes */
#include "ethernet.h"

/*! The size of the buffer in which the dynamic WEB page is created. */
#define webMAX_PAGE_SIZE	512

/*! Standard GET response. */
#define webHTTP_OK	"HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"

/*! The port on which we listen. */
#define webHTTP_PORT		( 80 )

/*! Delay on close error. */
#define webSHORT_DELAY		( 10 )

/*! Format of the dynamic page that is returned on each connection. */
#define webHTML_START \
"<html>\
<head>\
</head>\
<BODY onLoad=\"window.setTimeout(&quot;location.href='index.html'&quot;,1000)\" bgcolor=\"#FFFFFF\" text=\"#2477E6\">\
\r\nPage Hits = "

#define webHTML_END \
"\r\n</pre>\
\r\n</font></BODY>\
</html>"

portCHAR cDynamicPage[ webMAX_PAGE_SIZE ];
portCHAR cPageHits[ 11 ];


/****** */
#if 0
#include<asf.h>
#else
#include <adc.h>
#include <gpio.h>
/* Connection of the temperature sensor */
#  define EXAMPLE_ADC_TEMPERATURE_CHANNEL     0
#  define EXAMPLE_ADC_TEMPERATURE_PIN         AVR32_ADC_AD_0_PIN
#  define EXAMPLE_ADC_TEMPERATURE_FUNCTION    AVR32_ADC_AD_0_FUNCTION

/* Connection of the light sensor */
#  define EXAMPLE_ADC_LIGHT_CHANNEL           2
#  define EXAMPLE_ADC_LIGHT_PIN               AVR32_ADC_AD_2_PIN
#  define EXAMPLE_ADC_LIGHT_FUNCTION          AVR32_ADC_AD_2_FUNCTION

/* Connection of the potentiometer */
#  define EXAMPLE_ADC_POTENTIOMETER_CHANNEL   1
#  define EXAMPLE_ADC_POTENTIOMETER_PIN       AVR32_ADC_AD_1_PIN
#  define EXAMPLE_ADC_POTENTIOMETER_FUNCTION  AVR32_ADC_AD_1_FUNCTION

signed short adc_value_temp  = -1;
signed short adc_value_light = -1;
signed short adc_value_pot   = -1;

const gpio_map_t ADC_GPIO_MAP = {
	{EXAMPLE_ADC_TEMPERATURE_PIN, EXAMPLE_ADC_TEMPERATURE_FUNCTION},
	{EXAMPLE_ADC_LIGHT_PIN, EXAMPLE_ADC_LIGHT_FUNCTION},
	{EXAMPLE_ADC_POTENTIOMETER_PIN, EXAMPLE_ADC_POTENTIOMETER_FUNCTION}
};

void initADC(void);

void initADC(void) {
	/** GPIO pin/adc-function map. */
	
	
	/* Assign and enable GPIO pins to the ADC function. */
	gpio_enable_module(ADC_GPIO_MAP, sizeof(ADC_GPIO_MAP) /
	sizeof(ADC_GPIO_MAP[0]));
	
	AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
	adc_configure(&AVR32_ADC);

	/* Enable the ADC channels. */
	adc_enable(&AVR32_ADC, EXAMPLE_ADC_TEMPERATURE_CHANNEL);
	adc_enable(&AVR32_ADC, EXAMPLE_ADC_LIGHT_CHANNEL);
	adc_enable(&AVR32_ADC, EXAMPLE_ADC_POTENTIOMETER_CHANNEL);
}
#endif

/*********** */
/*! Function to process the current connection */
static void prvweb_ParseHTMLRequest( struct netconn *pxNetCon );


/*! \brief WEB server main task
 *         check for incoming connection and process it
 *
 *  \param pvParameters   Input. Not Used.
 *
 */
portTASK_FUNCTION( vBasicWEBServer, pvParameters );
portTASK_FUNCTION( vBasicWEBServer, pvParameters )
{
struct netconn *pxHTTPListener, *pxNewConnection;

	/* Create a new tcp connection handle */
	pxHTTPListener = netconn_new( NETCONN_TCP );
	netconn_bind(pxHTTPListener, NULL, webHTTP_PORT );
	netconn_listen( pxHTTPListener );
	
	/*** */
	initADC();
	/*** */
	
	
	/* Loop forever */
	for( ;; )
	{
#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
		/* Wait for a first connection. */
		pxNewConnection = netconn_accept(pxHTTPListener);
#else
        while(netconn_accept(pxHTTPListener, &pxNewConnection) != ERR_OK)
        {
            vTaskDelay( webSHORT_DELAY );
		}
#endif
		vParTestSetLED(webCONN_LED, pdTRUE);

		if(pxNewConnection != NULL)
		{
			prvweb_ParseHTMLRequest(pxNewConnection);
		}/* end if new connection */

		vParTestSetLED(webCONN_LED, pdFALSE);

	} /* end infinite loop */
}


/*! \brief parse the incoming request
 *         parse the HTML request and send file
 *
 *  \param pxNetCon   Input. The netconn to use to send and receive data.
 *
 */
static void prvweb_ParseHTMLRequest( struct netconn *pxNetCon )
{
struct netbuf *pxRxBuffer;
portCHAR *pcRxString;
unsigned portSHORT usLength;
static unsigned portLONG ulPageHits = 0;


#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
	/* We expect to immediately get data. */
	pxRxBuffer = netconn_recv( pxNetCon );
#else
    while(netconn_recv( pxNetCon, &pxRxBuffer) != ERR_OK)
    {
		vTaskDelay( webSHORT_DELAY );
	}
#endif
	if( pxRxBuffer != NULL )
	{
		/* Where is the data? */
		netbuf_data( pxRxBuffer, ( void * ) &pcRxString, &usLength );

/*** */
adc_start(&AVR32_ADC);
/*** */

		/* Is this a GET?  We don't handle anything else. */
		if(( NULL != pcRxString               )
		&& ( !strncmp( pcRxString, "GET", 3 ) ))
		{
			/* Update the hit count. */
			ulPageHits++;
			sprintf( cPageHits, "%d", (int)ulPageHits );

			/* Write out the HTTP OK header. */
			netconn_write( pxNetCon, webHTTP_OK, (u16_t) strlen( webHTTP_OK ), NETCONN_COPY );

			/* Generate the dynamic page... First the page header. */
			strcpy( cDynamicPage, webHTML_START );

			/* ... Then the hit count... */
			strcat( cDynamicPage, cPageHits );
			strcat( cDynamicPage, "<p><pre>Task          State  Priority  Stack	#<br>************************************************<br>" );

			/* ... Then the list of tasks and their status... */
			vTaskList( ( signed portCHAR * ) cDynamicPage + strlen( cDynamicPage ) );


			/****** */
			adc_value_temp = adc_get_value(&AVR32_ADC, EXAMPLE_ADC_TEMPERATURE_CHANNEL);
			adc_value_light = adc_get_value(&AVR32_ADC, EXAMPLE_ADC_LIGHT_CHANNEL);
			adc_value_pot = adc_get_value(&AVR32_ADC, EXAMPLE_ADC_POTENTIOMETER_CHANNEL);

			sprintf(cPageHits, "<p>t: %d", adc_value_temp);
			strcat( cDynamicPage, cPageHits);
			
			sprintf(cPageHits, "<p>l: %d", adc_value_light);
			strcat( cDynamicPage, cPageHits);
			
			sprintf(cPageHits, "<p>p: %d", adc_value_pot);
			strcat( cDynamicPage, cPageHits);
			/****** */


			/* ... Finally the page footer. */
			strcat( cDynamicPage, webHTML_END );

			/* Write out the dynamically generated page. */
			netconn_write( pxNetCon, cDynamicPage, (u16_t) strlen( cDynamicPage ), NETCONN_COPY );
		}
		netbuf_delete( pxRxBuffer );
	}

	netconn_close( pxNetCon );
	netconn_delete( pxNetCon );

}

#endif

