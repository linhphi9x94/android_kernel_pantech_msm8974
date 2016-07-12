rm ./zImage
rm ./dt.img
rm ./arch/arm/boot/dt.img
./dtbTool -o ./arch/arm/boot/dt.img -s 2048 -p ./scripts/dtc/ ./arch/arm/boot/
#cp ./arch/arm/boot/zImage ./
