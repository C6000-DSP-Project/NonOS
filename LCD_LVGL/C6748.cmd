/****************************************************************************/
/*                                                                          */
/*              OMAPL138 �� DSP C6748 �ڴ�ռ���䶨��                      */
/*                                                                          */
/*              2014��07��05��                                              */
/*                                                                          */
/****************************************************************************/
-heap  131072
-stack 65536

MEMORY
{
    DSPL2RAM     o = 0x00800000  l = 0x00040000      /* 256KB L2RAM */

    SHRAM        o = 0x80000000  l = 0x00020000      /* 128KB ���� RAM */
    DDR2         o = 0xC0000000  l = 0x08000000      /* 128MB DDR2 */
}

SECTIONS
{
    .text			>  DDR2                 		 /* ��ִ�д��� */
    .stack  		>  DSPL2RAM 				     /* ���ϵͳջ */

    .cio			>  DSPL2RAM                      /* C ����������� */
    .vectors		>  DSPL2RAM      				 /* �ж������� */
    .const			>  DSPL2RAM                      /* ���� */
    .data			>  DSPL2RAM                      /* �ѳ�ʼ��ȫ�ּ���̬���� */
    .switch			>  DSPL2RAM                      /* ��ת�� */
    .sysmem			>  DSPL2RAM                      /* ��̬�ڴ�������� */

    .pinit			>  DSPL2RAM                      /* C++ �ṹ�� */
    .cinit			>  DSPL2RAM                      /* ��ʼ���� */

    .init_array		>  DSPL2RAM
    .fardata		>  DSPL2RAM

	GROUP(NEARDP_DATA)
	{
	   .neardata
	   .rodata
	   .bss
	}               >  DSPL2RAM

    .far			>  DSPL2RAM

    .DDR2			>  DDR2
}
