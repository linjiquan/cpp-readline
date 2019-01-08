#pragma once

typedef enum _xma_status_
{
	XS_SUCCESS 		= 0,				/* Successful status         */
	XS_INTRNL_ERR 	= 1,				/* Internal error - buffers, memory */
	XS_QUIT			= 100,				/* Exit */
} XmaStatus;