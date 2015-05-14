##这个目录不归我管理，只负责上传最初版本做文件系统

L_EMV	=libemv.so
L_EMV_V	=2.0.0

L_SMC	=libSMC_CHK.so
L_SMC_V	=1.0.0

L_VIK	=libVikang.so
L_VIK_V	=1.0.0

L_QPB	=libvisawave.so
L_QPB_V	=2.0.0

.PHONY:total
total:
	cp ./$(L_EMV).$(L_EMV_V) ../rel/
	cp ./$(L_SMC).$(L_SMC_V) ../rel/
	cp ./$(L_VIK).$(L_VIK_V) ../rel/
	cp ./$(L_QPB).$(L_QPB_V) ../rel/

.PHONY:clean
clean:
	rm  -rf ../rel/$(L_EMV).*
	rm  -rf ../rel/$(L_SMC).*
	rm  -rf ../rel/$(L_VIK).*
	rm  -rf ../rel/$(L_QPB).*

