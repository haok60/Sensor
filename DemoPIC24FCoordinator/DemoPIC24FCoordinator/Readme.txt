1. Contact Information.
    Name: Nguyen Tien Dat.
    Class: KSTN - DTVT.
    Course: K54.
    Phone: 01649616832.
    University: Hanoi University of Technology.

2. Sofware.
    Name: DemoPIC24FCoordinator.
    Version: Primary.

/-----------------Date: 31/08/2012-----------------/
3. Describe.
    Thiet bi duoc nap file .hex sau khi bien dich ma nguon chuong trinh nay, sau
    khi duoc cap nguon se khoi tao mot mang theo chuan Zigbee va cho phep cac thiet
    bi khac gia nhap vao mang cua no. Trong version Primary, toi chi chinh sua mot
    vai cho tu ma nguon duoc cho boi Microchip sao cho phu hop voi vi dieu khien
    PIC24FJ128GA306 ma chua he co bat cu tich hop gi them.

4. Install.
    Step 1: Re-compile this source code.
    Step 2: Programmed .hex file to MCU in the target that designed by Lab411
    Step 3: Supply power to the target.

/-----------------Date: 20/11/2012-----------------/
5. Sua doi.
    Ma nguon da duoc chinh sua voi cac tinh nang sau:
    - gui ban tin dinh ki (chua cac du lieu nhiet do, do am, dien ap) ve may tinh nhung voi
      voi chu ki 20s. Chu ki nay co the thay doi bang viec thay doi gia tri trong file zigbee.def
    - gui ban tin theo yeu cau (bao gom du lieu nhiet do, do am, dien ap) ve may tinh nhung.
    - gui ban tin canh bao cac su kien (bao gom canh bao co chay, canh bao het nang luong)
    cac tinh nang tren duoc dinh thoi bang timer3:timer2 tao thanh bo dem thoi gian 32 bit,
    cho phep tao ngat voi chu ki toi da len toi hon 72 tieng.

/-----------------Date: 06/04/2013-----------------/
6. Tinh nang actor.
    - Coordinator quyet dinh duoc trien khai ket noi voi mach dieu khien bom tuoi.
    - Khi dien ap xuong duoi mot nguong, nguon cap cho diode zener va
      module ADC se duoc tat di de tiet kiem nang luong.