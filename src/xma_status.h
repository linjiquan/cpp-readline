#pragma once

#ifndef XMA_CASE_STR_BIGIN
#define XMA_CASE_STR_BIGIN(state) switch(state){                                                                                                   
#define XMA_CASE_STR(state) case state:return #state;break;
#define XMA_CASE_STR_END()  default:return "Unknown";break;}
#endif

typedef enum _xma_status_
{
	XS_SUCCESS 		  = 0,				/* Successful status         */
	XS_INTRNL_ERR 	= 1,				/* Internal error - buffers, memory */
	XS_UNKOWN_EVENT = 2,        /* Unknow event */
	XS_NO_SPACE     = 3,
	XS_INVALID_STATE= 4,
	XS_QUIT			    = 100,				/* Exit */
} XmaStatus;
