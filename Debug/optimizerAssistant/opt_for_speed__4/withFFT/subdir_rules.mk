################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
withFFT/withFFTmain.obj: ../withFFT/withFFTmain.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla0 --vcu_support=vcu0 --float_support=fpu32 -O3 --opt_for_speed=4 --include_path="D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/include" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/withZVD" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/withFFT" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/lib" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/ProjectHeader" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/AlgorithmInclude" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/AlgorithmSource" --include_path="J:/MyCode/CCS/workspace_v6_3/ChoicePhaseWithKindsV1.2/include" --advice:performance=all -g --c99 --relaxed_ansi --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="withFFT/withFFTmain.d" --obj_directory="withFFT" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


