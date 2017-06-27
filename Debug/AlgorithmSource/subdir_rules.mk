################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
AlgorithmSource/FrequencyCalutate.obj: ../AlgorithmSource/FrequencyCalutate.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla0 --vcu_support=vcu0 --float_support=fpu32 -O0 --opt_for_speed=1 --include_path="D:/Program Files (x86)/TI/ccsv6/tools/compiler/c2000_15.12.3.LTS/include" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/withZVD" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/withFFT" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/lib" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/ProjectHeader" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/AlgorithmInclude" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/AlgorithmSource" --include_path="F:/MyCode/CCS/workspace_v6_2/ChoicePhaseWithKindsV1.3/include" --advice:performance=all -g --c99 --relaxed_ansi --display_error_number --diag_warning=225 --preproc_with_compile --preproc_dependency="AlgorithmSource/FrequencyCalutate.d" --obj_directory="AlgorithmSource" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


