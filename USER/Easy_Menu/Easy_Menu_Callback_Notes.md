# Easy_Menu_User.c 回调函数记录

> **用途**：使用 Easy Menu Builder 图形化工具重新生成 `Easy_Menu_User.c` 后，参考此文档恢复回调函数体和手动修改。
>
> 工具会清空 `/* USER CODE BEGIN */` 到 `/* USER CODE END */` 之间的内容，以下所有代码都需要手动补回。

---

## 1. 文件头部声明（USER CODE PUBLIC 区域）

生成后在 `/* USER CODE PUBLIC BEGIN */` 和 `/* USER CODE PUBLIC END */` 之间添加：

```c
extern float ypr[3];
```

---

## 2. 结构体类型修正

工具可能生成错误的数据类型，需要手动修改 `Easy_Menu_Ui_Data` 结构体中的以下成员：

| 成员名 | 工具可能生成的类型 | 正确类型 |
|--------|-------------------|---------|
| `yaw` | `signed short int` | `float` |
| `pitch` | `unsigned char` | `float` |
| `roll` | `unsigned char` | `float` |

---

## 3. Init 函数中的绑定修正

### data_show 页面（工具生成后需手动替换）

工具生成的（错误）：
```c
Show_Item_Init(PAGE(data_show), ITEM(yaw_show), "yaw", SIGNED_SHORT_INT, &Easy_Menu_Ui_Data.yaw, 10, Yaw_Show_Callback);
Show_Item_Init(PAGE(data_show), ITEM(pitch_val), "pitch", UNSIGNED_CHAR, &Easy_Menu_Ui_Data.pitch, 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(roll_val), "roll", UNSIGNED_CHAR, &Easy_Menu_Ui_Data.roll, 10, NULL);
```

替换为（正确，直接绑定 ypr[] 数组）：
```c
Show_Item_Init(PAGE(data_show), ITEM(yaw_show),   "yaw",   FLOAT, &ypr[0], 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(pitch_val),  "pitch", FLOAT, &ypr[1], 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(roll_val),   "roll",  FLOAT, &ypr[2], 10, NULL);
Show_Item_Init(PAGE(data_show), ITEM(gray_binary), "gray",  UNSIGNED_SHORT_INT, &Easy_Menu_Ui_Data.gray_binary, 10, NULL);
```

---

## 4. 回调函数体

### Switch_Page 回调

```c
void Switch_Page__1__Item_Callback(unsigned char data)
{
    /* USER CODE BEGIN */
    LED_Set(data);
    /* USER CODE END */
}

void Switch_Page__2__Item_Callback(unsigned char data)
{
    /* USER CODE BEGIN */
    // LED2 扩展
    /* USER CODE END */
}

void Switch_Page__3__Item_Callback(unsigned char data)
{
    /* USER CODE BEGIN */
    // LED3 扩展
    /* USER CODE END */
}

void Switch_Page__4__Item_Callback(unsigned char data)
{
    /* USER CODE BEGIN */
    // LED4 扩展
    /* USER CODE END */
}
```

### 时钟 Show_Item 回调（User_Show_Page）

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

---

## 5. 已知问题：Easy Menu Builder 不保留 USER CODE 区域

当前版本的 Easy Menu Builder 重新生成时会清空所有 `USER CODE` 区域的内容。

**影响**：每次用工具重新生成后，上述所有回调函数体和声明都会丢失，需要手动恢复。

**建议**：
- 菜单结构确定后，不再使用工具重新生成
- 如需修改菜单结构，手动编辑 `Easy_Menu_User.c`
- 或等工具修复 USER CODE 保留功能后再使用

---

## 6. JSON 配置文件位置

```
C:\Users\17818\Desktop\Menu\User_Menu_Config.json
```

JSON 中的 `数据类型` 字段与 C 类型的对应关系：

| JSON 值 | C 类型 |
|---------|--------|
| `uint8_val` | `UNSIGNED_CHAR` |
| `uint16_val` | `UNSIGNED_SHORT_INT` |
| `uint32_val` | `UNSIGNED_INT` |
| `int8_val` | `SIGNED_CHAR` |
| `int16_val` | `SIGNED_SHORT_INT` |
| `int32_val` | `SIGNED_INT` |
| `float_val` | `FLOAT` |
