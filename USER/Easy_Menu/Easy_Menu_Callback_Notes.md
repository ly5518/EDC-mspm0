# Easy_Menu_User.c 回调函数记录

> **用途**：使用 Easy Menu Builder 图形化工具重新生成 `Easy_Menu_User.c` 后，参考此文档恢复回调函数体和手动修改。
>
> 工具会清空 `/* USER CODE BEGIN */` 到 `/* USER CODE END */` 之间的内容，以下所有代码都需要手动补回。

---

## 1. 文件头部声明（USER CODE PUBLIC 区域）

```c
extern float ypr[3];
```

---

## 2. 结构体成员（工具可能生成错误类型，需手动修正）

| 成员名 | 工具可能生成 | 正确类型 |
|--------|------------|---------|
| `yaw` | `signed short int` | `float` |
| `pitch` | `unsigned char` | `float` |
| `roll` | `unsigned char` | `float` |

新增成员（工具不会生成，需手动添加到结构体末尾）：

```c
unsigned short int analog_vals[8];  // 灰度 8 通道模拟量
char gray_bin_str[9];               // 灰度二进制字符串 "10100101"
unsigned short int white_ref[8];    // 白场校准基准
unsigned short int black_ref[8];    // 黑场校准基准
unsigned short int threshold[8];    // 各通道阈值
unsigned char cal_state;            // 校准状态：0=空闲 1=白场采集中 2=黑场采集中 3=完成
```

---

## 3. 页面/条目变量声明（工具不会生成，需手动添加）

```c
// data_show 页面
Ordinary_Page data_show;
    Show_Item yaw_show;
    Text_Item gray_bin_item;    // 工具可能生成为 Show_Item，需改为 Text_Item
    Show_Item pitch_val;
    Show_Item roll_val;

// gray 页面
Ordinary_Page gray;
    Text_Item gray_bin_display;
    Show_Item gray_ch[8];       // 工具不会生成数组，需手动声明
    Text_Item gray_white_cal;
    Text_Item gray_black_cal;
```

---

## 4. 条目数组

```c
Item *data_show_items[4] = {
    ITEM(yaw_show),
    ITEM(gray_bin_item),
    ITEM(pitch_val),
    ITEM(roll_val)
};

Item *gray_items[11] = {
    ITEM(gray_bin_display),
    ITEM(gray_ch[0]),
    ITEM(gray_ch[1]),
    ITEM(gray_ch[2]),
    ITEM(gray_ch[3]),
    ITEM(gray_ch[4]),
    ITEM(gray_ch[5]),
    ITEM(gray_ch[6]),
    ITEM(gray_ch[7]),
    ITEM(gray_white_cal),
    ITEM(gray_black_cal)
};
```

---

## 5. Init 函数中的绑定（工具生成后需手动替换）

### data_show 页面

工具生成的（错误）：
```c
Show_Item_Init(PAGE(data_show), ITEM(yaw_show), "yaw", SIGNED_SHORT_INT, &Easy_Menu_Ui_Data.yaw, 10, Yaw_Show_Callback);
Show_Item_Init(PAGE(data_show), ITEM(gray_binary), "gray_binary", UNSIGNED_CHAR, &Easy_Menu_Ui_Data.gray_binary, 10, Gray_Binary_Callback);
Show_Item_Init(PAGE(data_show), ITEM(pitch_val), "pitch", UNSIGNED_CHAR, &Easy_Menu_Ui_Data.pitch, 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(roll_val), "roll", UNSIGNED_CHAR, &Easy_Menu_Ui_Data.roll, 10, NULL);
```

替换为（正确）：
```c
Show_Item_Init(PAGE(data_show), ITEM(yaw_show),   "yaw",   FLOAT, &ypr[0], 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(pitch_val),  "pitch", FLOAT, &ypr[1], 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(roll_val),   "roll",  FLOAT, &ypr[2], 10, NULL);
Text_Item_Init(PAGE(data_show), ITEM(gray_bin_item), Easy_Menu_Ui_Data.gray_bin_str, NULL);
```

### gray 页面

工具不会生成此页面，需手动添加：
```c
Ordinary_Page_Init(PAGE(main_page), PAGE(gray), "Gray Sensor", gray_items, 11);
    Text_Item_Init(PAGE(gray), ITEM(gray_bin_display), Easy_Menu_Ui_Data.gray_bin_str, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[0]), "CH0", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[0], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[1]), "CH1", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[1], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[2]), "CH2", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[2], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[3]), "CH3", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[3], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[4]), "CH4", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[4], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[5]), "CH5", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[5], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[6]), "CH6", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[6], 10, NULL);
    Show_Item_Init(PAGE(gray), ITEM(gray_ch[7]), "CH7", UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.analog_vals[7], 10, NULL);
    Text_Item_Init(PAGE(gray), ITEM(gray_white_cal), "White Cal", Gray_White_Cal_Callback);
    Text_Item_Init(PAGE(gray), ITEM(gray_black_cal), "Black Cal", Gray_Black_Cal_Callback);
```

### main_page 的 Goto 条目

需手动添加（工具不会生成）：
```c
Goto_Item_Init(PAGE(main_page), ITEM(goto__data_show), "data_show", PAGE(data_show));
Goto_Item_Init(PAGE(main_page), ITEM(goto__gray), "gray", PAGE(gray));
```

main_page_items 数组需从 7 扩展到 9，添加 `ITEM(goto__data_show)` 和 `ITEM(goto__gray)`。

---

## 6. 回调函数体

### Switch_Page 回调

```c
void Switch_Page__1__Item_Callback(unsigned char data)
{
    /* USER CODE BEGIN */
    LED_Set(data);
    /* USER CODE END */
}

void Switch_Page__2__Item_Callback(unsigned char data) { }
void Switch_Page__3__Item_Callback(unsigned char data) { }
void Switch_Page__4__Item_Callback(unsigned char data) { }
```

### 时钟回调（User_Show_Page）

```c
void User_Show_Page__1__Item_Callback(void)
{
    if(Easy_Menu_Ui_Data.hours >= 24) Easy_Menu_Ui_Data.hours = 0;
}

void User_Show_Page__2__Item_Callback(void)
{
    if(Easy_Menu_Ui_Data.minutes >= 60)
    { Easy_Menu_Ui_Data.minutes = 0; Easy_Menu_Ui_Data.hours++; }
}

void User_Show_Page__3__Item_Callback(void)
{
    if(Easy_Menu_Ui_Data.seconds >= 60)
    { Easy_Menu_Ui_Data.seconds = 0; Easy_Menu_Ui_Data.minutes++; }
}

void User_Show_Page__4__Item_Callback(void)
{
    static unsigned int last_tick = 0;
    Easy_Menu_Ui_Data.millisecond += (easy_menu.tick - last_tick) / 10;
    last_tick = easy_menu.tick;
    if(Easy_Menu_Ui_Data.millisecond >= 100)
    { Easy_Menu_Ui_Data.millisecond = 0; Easy_Menu_Ui_Data.seconds++; }
}
```

### 灰度校准回调

```c
extern void Gray_White_Cal_Handler(void);
extern void Gray_Black_Cal_Handler(void);

void Gray_White_Cal_Callback(char *str) { Gray_White_Cal_Handler(); }
void Gray_Black_Cal_Callback(char *str) { Gray_Black_Cal_Handler(); }
```

---

## 7. Gray_Calibrate_Handler 实现（在 empty.c 中）

```c
#define CAL_SAMPLE_NUM  100

extern void Get_Analog_value(unsigned short *result);

static void Gray_Collect_Average(unsigned short *result)
{
    unsigned int sum[8] = {0};
    unsigned short buf[8];
    for (int n = 0; n < CAL_SAMPLE_NUM; n++)
    {
        Get_Analog_value(buf);
        for (int i = 0; i < 8; i++)
            sum[i] += buf[i];
        delay_us(500);
    }
    for (int i = 0; i < 8; i++)
        result[i] = sum[i] / CAL_SAMPLE_NUM;
}

static void Gray_Calc_Threshold(void)
{
    for (int i = 0; i < 8; i++)
    {
        if (Easy_Menu_Ui_Data.white_ref[i] > 0 && Easy_Menu_Ui_Data.black_ref[i] > 0)
            Easy_Menu_Ui_Data.threshold[i] = (Easy_Menu_Ui_Data.white_ref[i] + 2 * Easy_Menu_Ui_Data.black_ref[i]) / 3;
        else
            Easy_Menu_Ui_Data.threshold[i] = 2000;
    }
}

void Gray_White_Cal_Handler(void)
{
    Easy_Menu_Ui_Data.cal_state = 1;
    Gray_Collect_Average(Easy_Menu_Ui_Data.white_ref);
    Gray_Calc_Threshold();
    Easy_Menu_Ui_Data.cal_state = 3;
}

void Gray_Black_Cal_Handler(void)
{
    Easy_Menu_Ui_Data.cal_state = 2;
    Gray_Collect_Average(Easy_Menu_Ui_Data.black_ref);
    Gray_Calc_Threshold();
    Easy_Menu_Ui_Data.cal_state = 3;
}
```

---

## 8. 主循环灰度采集逻辑（在 empty.c 中）

```c
// 每 100ms（print_flag 触发）
No_Mcu_Ganv_Sensor_Task_Without_tick(&gray_sensor);
Get_Anolog_Value(&gray_sensor, gray_analog);

unsigned char digital = 0;
for (int i = 0; i < 8; i++)
{
    Easy_Menu_Ui_Data.analog_vals[i] = gray_analog[i];
    unsigned short th = (Easy_Menu_Ui_Data.threshold[i] > 0) ?
                         Easy_Menu_Ui_Data.threshold[i] : 2000;
    if (gray_analog[i] > th)
        digital |= (1 << i);
}
Easy_Menu_Ui_Data.gray_binary = digital;

for (int i = 0; i < 8; i++)
    Easy_Menu_Ui_Data.gray_bin_str[i] = (digital & (1 << (7 - i))) ? '1' : '0';
Easy_Menu_Ui_Data.gray_bin_str[8] = '\0';

Easy_Menu_Display_Refresh();
```

---

## 9. JSON 配置文件位置

```
C:\Users\17818\Desktop\Menu\User_Menu_Config.json
```

---

## 10. 已知问题

- Easy Menu Builder 重新生成时会清空 `USER CODE` 区域
- 工具不会生成 `data_show` 和 `gray` 页面的变量声明、条目数组、Init 绑定
- 结构体类型（yaw/pitch/roll）每次生成都会回到错误类型
- `analog_vals[8]`、`gray_bin_str[9]`、`white_ref[8]` 等成员每次生成都会丢失
