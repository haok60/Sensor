#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../Microchip/Common/delay.c ../../Microchip/Common/SHT1x.c ../PTC06.c ../../Microchip/Common/sralloc.c ../../Microchip/Common/Console.c ../../Microchip/Common/MSPI.c ../../Microchip/ZigBeeStack/SymbolTime.c ../../Microchip/ZigBeeStack/zAPS.c ../../Microchip/ZigBeeStack/ZigbeeTasks.c ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c ../../Microchip/ZigBeeStack/zNVM.c ../../Microchip/ZigBeeStack/zNWK.c ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c ../myZigBee.c ../../Microchip/ZigBeeStack/zZDO.c ../Router.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/385479254/delay.o ${OBJECTDIR}/_ext/385479254/SHT1x.o ${OBJECTDIR}/_ext/1472/PTC06.o ${OBJECTDIR}/_ext/385479254/sralloc.o ${OBJECTDIR}/_ext/385479254/Console.o ${OBJECTDIR}/_ext/385479254/MSPI.o ${OBJECTDIR}/_ext/1163520513/SymbolTime.o ${OBJECTDIR}/_ext/1163520513/zAPS.o ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o ${OBJECTDIR}/_ext/1163520513/zNVM.o ${OBJECTDIR}/_ext/1163520513/zNWK.o ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o ${OBJECTDIR}/_ext/1472/myZigBee.o ${OBJECTDIR}/_ext/1163520513/zZDO.o ${OBJECTDIR}/_ext/1472/Router.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/385479254/delay.o.d ${OBJECTDIR}/_ext/385479254/SHT1x.o.d ${OBJECTDIR}/_ext/1472/PTC06.o.d ${OBJECTDIR}/_ext/385479254/sralloc.o.d ${OBJECTDIR}/_ext/385479254/Console.o.d ${OBJECTDIR}/_ext/385479254/MSPI.o.d ${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d ${OBJECTDIR}/_ext/1163520513/zAPS.o.d ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d ${OBJECTDIR}/_ext/1163520513/zNVM.o.d ${OBJECTDIR}/_ext/1163520513/zNWK.o.d ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d ${OBJECTDIR}/_ext/1472/myZigBee.o.d ${OBJECTDIR}/_ext/1163520513/zZDO.o.d ${OBJECTDIR}/_ext/1472/Router.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/385479254/delay.o ${OBJECTDIR}/_ext/385479254/SHT1x.o ${OBJECTDIR}/_ext/1472/PTC06.o ${OBJECTDIR}/_ext/385479254/sralloc.o ${OBJECTDIR}/_ext/385479254/Console.o ${OBJECTDIR}/_ext/385479254/MSPI.o ${OBJECTDIR}/_ext/1163520513/SymbolTime.o ${OBJECTDIR}/_ext/1163520513/zAPS.o ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o ${OBJECTDIR}/_ext/1163520513/zNVM.o ${OBJECTDIR}/_ext/1163520513/zNWK.o ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o ${OBJECTDIR}/_ext/1472/myZigBee.o ${OBJECTDIR}/_ext/1163520513/zZDO.o ${OBJECTDIR}/_ext/1472/Router.o

# Source Files
SOURCEFILES=../../Microchip/Common/delay.c ../../Microchip/Common/SHT1x.c ../PTC06.c ../../Microchip/Common/sralloc.c ../../Microchip/Common/Console.c ../../Microchip/Common/MSPI.c ../../Microchip/ZigBeeStack/SymbolTime.c ../../Microchip/ZigBeeStack/zAPS.c ../../Microchip/ZigBeeStack/ZigbeeTasks.c ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c ../../Microchip/ZigBeeStack/zNVM.c ../../Microchip/ZigBeeStack/zNWK.c ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c ../myZigBee.c ../../Microchip/ZigBeeStack/zZDO.c ../Router.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ128GA306
MP_LINKER_FILE_OPTION=,--script=p24FJ128GA306.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/385479254/delay.o: ../../Microchip/Common/delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/delay.c  -o ${OBJECTDIR}/_ext/385479254/delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/delay.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/SHT1x.o: ../../Microchip/Common/SHT1x.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/SHT1x.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/SHT1x.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/SHT1x.c  -o ${OBJECTDIR}/_ext/385479254/SHT1x.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/SHT1x.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/SHT1x.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/PTC06.o: ../PTC06.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PTC06.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PTC06.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PTC06.c  -o ${OBJECTDIR}/_ext/1472/PTC06.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/PTC06.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/PTC06.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/sralloc.o: ../../Microchip/Common/sralloc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/sralloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/sralloc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/sralloc.c  -o ${OBJECTDIR}/_ext/385479254/sralloc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/sralloc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/sralloc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/Console.o: ../../Microchip/Common/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/Console.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/Console.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/Console.c  -o ${OBJECTDIR}/_ext/385479254/Console.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/Console.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/Console.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/MSPI.o: ../../Microchip/Common/MSPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/MSPI.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/MSPI.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/MSPI.c  -o ${OBJECTDIR}/_ext/385479254/MSPI.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/MSPI.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/MSPI.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/SymbolTime.o: ../../Microchip/ZigBeeStack/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/SymbolTime.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/SymbolTime.c  -o ${OBJECTDIR}/_ext/1163520513/SymbolTime.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zAPS.o: ../../Microchip/ZigBeeStack/zAPS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zAPS.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zAPS.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zAPS.c  -o ${OBJECTDIR}/_ext/1163520513/zAPS.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zAPS.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zAPS.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o: ../../Microchip/ZigBeeStack/ZigbeeTasks.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/ZigbeeTasks.c  -o ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o: ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zNVM.o: ../../Microchip/ZigBeeStack/zNVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNVM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNVM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zNVM.c  -o ${OBJECTDIR}/_ext/1163520513/zNVM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zNVM.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zNVM.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zNWK.o: ../../Microchip/ZigBeeStack/zNWK.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNWK.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNWK.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zNWK.c  -o ${OBJECTDIR}/_ext/1163520513/zNWK.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zNWK.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zNWK.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o: ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o: ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/myZigBee.o: ../myZigBee.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/myZigBee.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/myZigBee.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../myZigBee.c  -o ${OBJECTDIR}/_ext/1472/myZigBee.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/myZigBee.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/myZigBee.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zZDO.o: ../../Microchip/ZigBeeStack/zZDO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zZDO.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zZDO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zZDO.c  -o ${OBJECTDIR}/_ext/1163520513/zZDO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zZDO.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zZDO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Router.o: ../Router.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Router.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Router.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Router.c  -o ${OBJECTDIR}/_ext/1472/Router.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Router.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Router.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/385479254/delay.o: ../../Microchip/Common/delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/delay.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/delay.c  -o ${OBJECTDIR}/_ext/385479254/delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/delay.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/SHT1x.o: ../../Microchip/Common/SHT1x.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/SHT1x.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/SHT1x.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/SHT1x.c  -o ${OBJECTDIR}/_ext/385479254/SHT1x.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/SHT1x.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/SHT1x.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/PTC06.o: ../PTC06.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PTC06.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PTC06.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../PTC06.c  -o ${OBJECTDIR}/_ext/1472/PTC06.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/PTC06.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/PTC06.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/sralloc.o: ../../Microchip/Common/sralloc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/sralloc.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/sralloc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/sralloc.c  -o ${OBJECTDIR}/_ext/385479254/sralloc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/sralloc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/sralloc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/Console.o: ../../Microchip/Common/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/Console.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/Console.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/Console.c  -o ${OBJECTDIR}/_ext/385479254/Console.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/Console.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/Console.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/385479254/MSPI.o: ../../Microchip/Common/MSPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/385479254" 
	@${RM} ${OBJECTDIR}/_ext/385479254/MSPI.o.d 
	@${RM} ${OBJECTDIR}/_ext/385479254/MSPI.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/Common/MSPI.c  -o ${OBJECTDIR}/_ext/385479254/MSPI.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/385479254/MSPI.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/385479254/MSPI.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/SymbolTime.o: ../../Microchip/ZigBeeStack/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/SymbolTime.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/SymbolTime.c  -o ${OBJECTDIR}/_ext/1163520513/SymbolTime.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/SymbolTime.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zAPS.o: ../../Microchip/ZigBeeStack/zAPS.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zAPS.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zAPS.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zAPS.c  -o ${OBJECTDIR}/_ext/1163520513/zAPS.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zAPS.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zAPS.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o: ../../Microchip/ZigBeeStack/ZigbeeTasks.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/ZigbeeTasks.c  -o ${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/ZigbeeTasks.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o: ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zMAC_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zMAC_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zNVM.o: ../../Microchip/ZigBeeStack/zNVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNVM.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNVM.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zNVM.c  -o ${OBJECTDIR}/_ext/1163520513/zNVM.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zNVM.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zNVM.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zNWK.o: ../../Microchip/ZigBeeStack/zNWK.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNWK.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zNWK.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zNWK.c  -o ${OBJECTDIR}/_ext/1163520513/zNWK.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zNWK.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zNWK.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o: ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zPHY_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zPHY_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o: ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zSecurity_MRF24J40.c  -o ${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zSecurity_MRF24J40.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/myZigBee.o: ../myZigBee.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/myZigBee.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/myZigBee.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../myZigBee.c  -o ${OBJECTDIR}/_ext/1472/myZigBee.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/myZigBee.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/myZigBee.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1163520513/zZDO.o: ../../Microchip/ZigBeeStack/zZDO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1163520513" 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zZDO.o.d 
	@${RM} ${OBJECTDIR}/_ext/1163520513/zZDO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Microchip/ZigBeeStack/zZDO.c  -o ${OBJECTDIR}/_ext/1163520513/zZDO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1163520513/zZDO.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1163520513/zZDO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Router.o: ../Router.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Router.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Router.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Router.c  -o ${OBJECTDIR}/_ext/1472/Router.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Router.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -O1 -I".." -I"../../ZigBeeStack" -I"../../Common" -I"../../microchip/Common" -I"../../../microchip/ZigBeeStack" -I"../../../DemoPIC24FRouter" -I"../../microchip/ZigBeeStack" -I"../../Microchip/Common" -I"../../Microchip/ZigBeeStack" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Router.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81B -mreserve=data@0x81C:0x81D -mreserve=data@0x81E:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x827 -mreserve=data@0x82A:0x84F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--heap=1000,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="../../../../../../../../Program Files/Microchip/MPLAB C30/lib",--no-force-link,--smart-io,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--heap=1000,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--library-path="../../../../../../../../Program Files/Microchip/MPLAB C30/lib",--no-force-link,--smart-io,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/DemoPIC24FRouter.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
