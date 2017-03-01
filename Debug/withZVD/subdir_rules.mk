################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
withZVD/SoftOverZero.obj: ../withZVD/SoftOverZero.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"E:/Program Files (x86)/Ti/CCS6/ccsv6/tools/compiler/c2000_6.2.7/bin/cl2000" -v28 -ml -mt --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 -O3 --opt_for_speed=1 --include_path="E:/Program Files (x86)/Ti/CCS6/ccsv6/tools/compiler/c2000_6.2.7/include" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/withZVD" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/withFFT" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/lib" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/ProjectHeader" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/include" -g --relaxed_ansi --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="withZVD/SoftOverZero.pp" --obj_directory="withZVD" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

withZVD/mainZvd.obj: ../withZVD/mainZvd.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C2000 Compiler'
	"E:/Program Files (x86)/Ti/CCS6/ccsv6/tools/compiler/c2000_6.2.7/bin/cl2000" -v28 -ml -mt --cla_support=cla0 --float_support=fpu32 --vcu_support=vcu0 -O3 --opt_for_speed=1 --include_path="E:/Program Files (x86)/Ti/CCS6/ccsv6/tools/compiler/c2000_6.2.7/include" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/withZVD" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/withFFT" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/lib" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/ProjectHeader" --include_path="G:/MyCode/workspace_v6_0/ChoicePhaseWithKinds/include" -g --relaxed_ansi --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="withZVD/mainZvd.pp" --obj_directory="withZVD" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


