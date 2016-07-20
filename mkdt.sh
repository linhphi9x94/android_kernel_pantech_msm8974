rm ./zImage
rm ./dt.img
rm ./arch/arm/boot/dt.img
./dtbTool -o ./obj/KERNEL_OBJ/arch/arm/boot/dt.img -s 2048 -p ./obj/KERNEL_OBJ/scripts/dtc/ ./obj/KERNEL_OBJ/arch/arm/boot/
cp ./obj/KERNEL_OBJ/arch/arm/boot/zImage ./
cp ./obj/KERNEL_OBJ/arch/arm/boot/dt.img ./
