#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := MXOS_$(NET)_Interface

GLOBAL_INCLUDES += .
					
$(NAME)_SOURCES := mxos_socket.c mxos_network.c