rm ./zImage
rm ./dt.img
<<<<<<< HEAD
./dtbTool -o ./dt.img -s 2048 -p ./scripts/dtc/ ./arch/arm/boot/
cp ./arch/arm/boot/zImage ./
=======
rm ./arch/arm/boot/dt.img
./dtbTool -o ./arch/arm/boot/dt.img -s 2048 -p ./scripts/dtc/ ./arch/arm/boot/
#cp ./arch/arm/boot/zImage ./
>>>>>>> sunghun/cm-13.0_LA.BF.1.1.3-01610-8x74.0
