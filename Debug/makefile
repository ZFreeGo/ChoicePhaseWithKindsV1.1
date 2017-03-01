################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./AlgorithmSource/FrequencyCalutate.obj" \
"./ProjectSource/BasicModule.obj" \
"./ProjectSource/ConfigADC.obj" \
"./ProjectSource/DeviceIO.obj" \
"./ProjectSource/MonitorCalculate.obj" \
"./ProjectSource/PwmTimer.obj" \
"./ProjectSource/SampleProcess.obj" \
"./source/F2806x_Adc.obj" \
"./source/F2806x_CodeStartBranch.obj" \
"./source/F2806x_CpuTimers.obj" \
"./source/F2806x_DefaultIsr.obj" \
"./source/F2806x_DisInt.obj" \
"./source/F2806x_GlobalVariableDefs.obj" \
"./source/F2806x_OscComp.obj" \
"./source/F2806x_PieCtrl.obj" \
"./source/F2806x_PieVect.obj" \
"./source/F2806x_Sci.obj" \
"./source/F2806x_SysCtrl.obj" \
"./source/F2806x_usDelay.obj" \
"./withFFT/withFFTmain.obj" \
"../cmd/F28069.cmd" \
"../cmd/F2806x_Headers_nonBIOS.cmd" \
"../lib/2806x_BootROM_API_TABLE_Symbols.lib" \
"../lib/2806x_BootROM_API_TABLE_Symbols_fpu32.lib" \
"../lib/C28x_FPU_Lib.lib" \
"../lib/CLAmath_fpu32.lib" \
"../lib/SFO_TI_Build_V6b.lib" \
"../lib/SFO_TI_Build_V6b_FPU.lib" \
"../lib/rts2800_fpu32_fast_supplement.lib" \
$(GEN_CMDS__FLAG) \
-lrts2800_fpu32.lib \
-l"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/lib/rts2800_fpu32_fast_supplement.lib" \
-l"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/lib/C28x_FPU_Lib.lib" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include AlgorithmSource/subdir_vars.mk
-include ProjectSource/subdir_vars.mk
-include cmd/subdir_vars.mk
-include lib/subdir_vars.mk
-include source/subdir_vars.mk
-include withFFT/subdir_vars.mk
-include AlgorithmSource/subdir_rules.mk
-include ProjectSource/subdir_rules.mk
-include cmd/subdir_rules.mk
-include lib/subdir_rules.mk
-include source/subdir_rules.mk
-include withFFT/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
ChoicePhaseWithKindsV1.1.out \

EXE_OUTPUTS__QUOTED += \
"ChoicePhaseWithKindsV1.1.out" \

BIN_OUTPUTS += \
ChoicePhaseWithKindsV1.1.hex \

BIN_OUTPUTS__QUOTED += \
"ChoicePhaseWithKindsV1.1.hex" \


# All Target
all: ChoicePhaseWithKindsV1.1.out

# Tool invocations
ChoicePhaseWithKindsV1.1.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C2000 Linker'
	"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla0 --vcu_support=vcu0 --float_support=fpu32 -O3 --opt_for_speed=1 --advice:performance=all -g --relaxed_ansi --display_error_number --diag_warning=225 -z -m"ChoicePhaseWithKindsV1.1.map" --stack_size=0x400 --warn_sections -i"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/lib" -i"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/include" -i"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/lib" -i"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/ProjectSource" -i"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/ProjectHeader" -i"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/source" -i"J:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.1/include" --priority --reread_libs --display_error_number --xml_link_info="ChoicePhaseWithKindsV1.1_linkInfo.xml" --rom_model -o "ChoicePhaseWithKindsV1.1.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

ChoicePhaseWithKindsV1.1.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: C2000 Hex Utility'
	"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/hex2000"  -o "ChoicePhaseWithKindsV1.1.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "AlgorithmSource\FrequencyCalutate.d" "ProjectSource\BasicModule.d" "ProjectSource\ConfigADC.d" "ProjectSource\MonitorCalculate.d" "ProjectSource\PwmTimer.d" "ProjectSource\SampleProcess.d" "source\F2806x_Adc.d" "source\F2806x_CpuTimers.d" "source\F2806x_DefaultIsr.d" "source\F2806x_GlobalVariableDefs.d" "source\F2806x_OscComp.d" "source\F2806x_PieCtrl.d" "source\F2806x_PieVect.d" "source\F2806x_Sci.d" "source\F2806x_SysCtrl.d" "withFFT\withFFTmain.d" 
	-$(RM) "ProjectSource\DeviceIO.d" 
	-$(RM) "AlgorithmSource\FrequencyCalutate.obj" "ProjectSource\BasicModule.obj" "ProjectSource\ConfigADC.obj" "ProjectSource\DeviceIO.obj" "ProjectSource\MonitorCalculate.obj" "ProjectSource\PwmTimer.obj" "ProjectSource\SampleProcess.obj" "source\F2806x_Adc.obj" "source\F2806x_CodeStartBranch.obj" "source\F2806x_CpuTimers.obj" "source\F2806x_DefaultIsr.obj" "source\F2806x_DisInt.obj" "source\F2806x_GlobalVariableDefs.obj" "source\F2806x_OscComp.obj" "source\F2806x_PieCtrl.obj" "source\F2806x_PieVect.obj" "source\F2806x_Sci.obj" "source\F2806x_SysCtrl.obj" "source\F2806x_usDelay.obj" "withFFT\withFFTmain.obj" 
	-$(RM) "source\F2806x_CodeStartBranch.d" "source\F2806x_DisInt.d" "source\F2806x_usDelay.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

