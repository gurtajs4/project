/*=======================================================================
  
       Copyright(c) 2009, Works Systems, Inc. All rights reserved.
  
       This software is supplied under the terms of a license agreement 
       with Works Systems, Inc, and may not be copied nor disclosed except 
       in accordance with the terms of that agreement.
  
  =======================================================================*/
#ifndef __WAR_LOG_H
#define __WAR_LOG_H

#include <android/log.h>
void war_pre_log(unsigned int level, const char *function, char *buffer);
#endif
