
 1. director dtructure
 sample driver
��  tcbd.h
��  tcbd.c
��  Makefile
��  
 SDK
����src
��  ��  tcbd_drv_io.c
��  ��  tcbd_drv_rf.c
��  ��  tcbd_drv_ip.c
��  ��  tcbd_hal.c
��  ��  tcbd_drv_peri.c
��  ��  tcbd_drv_dual.c
��  ��  tcbd_api_common.c
��  ��  Makefile
��  ��  
��  ����tcpal_linux
��  ��      tcpal_linux.c
��  ��      tcpal_io_i2c.c
��  ��      tcpal_queue.c
��  ��      tcpal_io_cspi.c
��  ��      tcpal_debug.c
��  ��      tcpal_irq_handler.c
��  ��      Makefile
��  ��      
��  ����tcbd_stream_parser
��  ��      tcbd_stream_parser.c
��  ��      Makefile
��  ��      
��  ����tcbd_test
��  ��      tcbd_test_io.c
��  ��      Makefile
��  ��      
��  ����tcbd_diagnosis
��          tcbd_diagnosis.c
��          Makefile
��          
����inc
    ��  tcbd_drv_register.h
    ��  tcbd_api_common.h
    ��  tcbd_drv_io.h
    ��  tcbd_drv_rf.h
    ��  tcbd_drv_ip.h
    ��  tcbd_hal.h
    ��  tcbd_error.h
    ��  tcbd_feature.h
    ��  TCC317X_BOOT_TDMB.h
    ��  
    ����tcbd_stream_parser
    ��      tcbd_stream_parser.h
    ��      
    ����tcpal
    ��      tcpal_debug.h
    ��      tcpal_os.h
    ��      tcpal_queue.h
    ��      tcpal_types.h
    ��      
    ����tcbd_diagnosis
            tcbd_diagnosis.h
 
 2. porting guide
 tcbd.c ������ SDK�� �̿��Ͽ� ������� ���� driver�Դϴ�. �ش� ������ ���� �Ͻ�
 �� ���� ������ �� ���� ���Դϴ�. ���ý� �����Ǿ���Һκ��� ������ �����ϴ�. 


 - inc/tcbd_feature.h
 	SDK�� �������� ������ �Ҽ��ֵ��� ���� feature�� ���ǵǾ� �ֽ��ϴ�.

 - tcbd_hal.c  
 	CPU�� GPIO������ ��ġ�� �����Դϴ�. �ϵ���� ������ �µ��� ���� �Ǿ��
	�մϴ�. ����� telechips EVBȯ�濡 �µ��� �Ǿ� �ֽ��ϴ�. __USE_TC_CPU__
	�� �˻��ϸ� �ش� �κ��� Ȯ�� �� �� �ֽ��ϴ�.

 - tcpal_linux/tcpal_io_cspi.c 
	SPI I/O���� �Լ��� ��ġ�� �����Դϴ�. ���� �ڷ�Ĩ�� BSP���� �����ϵ���
	�ۼ��Ǿ� �ֽ��ϴ�.(__USE_TC_CPU__ �� �˻�) ��κ� kernel API�� ����Ͽ�
	���Ƿ� ũ�� ������ ���� ���� ���Դϴ�. __USE_TC_CPU__�κи� ���� �Ͻð�,
	spi prove �Լ����� spi_setup�Լ��� ȣ���ϸ� �ɰ��Դϴ�. ���� driver init
	�Լ� �Ǵ� prove�Լ����� tcpal_set_cspi_io_function�� ȣ�����־�� �մϴ�
	. �ش� �Լ����� ���õ� �Լ� �����Ͱ� �ʱ�ȭ �ǹǷ� �ش� �Լ��� ȣ����
	���� ������ driver�� ���������� �������� ���� ���Դϴ�.

 - tcpal_linux/tcpal_io_i2c.c 
 	I2C I/O���� �Լ����� ��ġ�� �����Դϴ�. tcpal_io_cspi.c�� ���� kernel
	API�θ� �ۼ��Ǿ� �����Ƿ� Ư���� ������ ���� ���� ���Դϴ�. driver init
	�Լ����� tcpal_set_i2c_io_function�� ȣ�����־�� �մϴ�.

 - tcpal_linux/tcpal_irq_handler.c 
	���ͷ�Ʈó�� ���� �Լ��� ��ġ�� �����Դϴ�. ���� ���� ����̹��� �µ���
	�Ǿ� �ֽ��ϴ�. I2C/STS�� ����ϴ� ���� SPI�� ����ϴ� ��찡 �ٸ��� 
	�����ϵ��� �Ǿ� ������ __CSPI_ONLY__, __I2C_STS__ feature�� ���Ͽ� ����
	�� �� �ֽ��ϴ�. 

	__CSPI_ONLY__��쿡�� MSC data�� 4byte ����� CIF���� �ٿ��� �����մϴ�.
	SDK������ �װ��� �Ľ��Ͽ��� callback�Լ��� �����ϵ��� �ۼ��Ǿ� �ֽ��ϴ�.
	���� tcpal_irq_stream_callback�Լ����� �Ľ̵Ǿ� ������ ��Ʈ���� ������
	���۸��Ͽ� ����ϸ� �˴ϴ�. __MERGE_EACH_TYPEOF_STREAM__�� �����ϸ� ����
	��Ʈ �ѹ��� ���Ե� �������� CIF�� �����Ͽ� callback�Լ��� �ѹ��� ȣ���
	�ϴ�. __MERGE_EACH_TYPEOF_STREAM__�� �������� ������ CIF���� callback��
	���� ȣ��˴ϴ�. ����͸� �����ʹ� ��Ʈ���� �����ϰų� �������Ϳ��� ����
	�� ������ �⺻������ ��Ʈ���� ���Եǵ��� ���� �Ǿ� �ֽ��ϴ�. �����ڵ带 
	�����Ͻʽÿ�.

	__I2C_STS__�� ��� ���ͷ�Ʈ�� FIC�����͸� �д� ��쿡�� ���Ǹ�, MSC��
	���ʹ� TSIF�� ���Ͽ� 188 bytes������ ���۵˴ϴ�. ����͸� �����ʹ� spi��
	�޸� ��Ʈ���� ���ԵǾ� ���� ������, API�� ����Ͽ� register�� �о�� ��
	�ϴ�. ���� �ڵ带 �����Ͻʽÿ�.

	���˽�(���ļ��� �����Ͽ��� ���) FIC data�� ������, ���񽺸� �����ϸ�
	FIC�� ���̻� ������ �ʰ� MSC data�� �����Ե˴ϴ�. ���� ���ý� FIC data
	�� ������ feature(__ALWAYS_FIC_ON__)�������� on/off�� �� �ֽ��ϴ�. 

	driver init�Լ� �Ǵ� prove�Լ����� tcpal_irq_register_handler�Լ��� �ݵ�
	�� ȣ���� �־�� interrupt handler�� ���������� ��ϵ˴ϴ�. �׸���
	tcpal_irq_register_handler���� tcbd_irq_handler_data.tcbd_irq�κ���
	�ش� H/W�� �°� �����ؾ� �մϴ�.
