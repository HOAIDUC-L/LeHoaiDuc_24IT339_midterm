# Báo cáo Giữa kỳ: Cài đặt lệnh `ls(1)` UNIX tinh giản bằng ngôn ngữ C

---

## 1. Thông tin sinh viên & Repository

* **Họ và tên:** Lê Hoài Đức
* **Mã sinh viên:** 24IT339
* **Học phần:** Linux & Phần mềm mã nguồn mở / Lập trình hệ thống nâng cao
* **Đề tài:** Dự án giữa kỳ – Xây dựng công cụ UNIX `ls(1)` từ con số 0 (from scratch) bằng C
* **Tên Repository:** `LeHoaiDuc_24IT339_midterm`
* **Link GitHub:** [https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm](https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm)

---

## 2. Mô tả tổng quan dự án

Dự án này là một phiên bản cài đặt hoàn chỉnh, độc lập và chuẩn mực của tiện ích dòng lệnh kinh điển UNIX `ls(1)`, được lập trình hoàn toàn bằng ngôn ngữ C (chuẩn C99 kết hợp hệ thống API POSIX.1-2008). Chương trình không sử dụng bất kỳ thư viện ngoài không chuẩn nào và không phụ thuộc vào các tiện ích mở rộng riêng của GNU.

### Phạm vi và Đặc tả kỹ thuật
Chương trình được thiết kế bám sát tuyệt đối theo **tài liệu hướng dẫn (manual page) NetBSD 10.1 `ls(1)`** được cung cấp (`ls [ -AacdFfhiklnqRrSstuw] [file ...]`). Theo đúng tôn chỉ đề bài:
* **Đặc tả duy nhất:** Chỉ triển khai chính xác các tùy chọn, hành vi, định dạng hiển thị và quy tắc kết hợp/ghi đè được quy định trong manual page được giao.
* **Định dạng mặc định:** Xuất danh sách tập tin theo quy tắc mỗi mục trên một dòng (*"By default, ls lists one entry per line to standard output"*).
* **Không phỏng đoán mở rộng:** Tuyệt đối không thêm các cờ ngoài tài liệu (như hiển thị nhiều cột kiểu GNU khi ra terminal, đổi màu font chữ, các cờ `-1`, `-C`, `-m`, `--color`,...).
* **Độ bền bỉ & xử lý lỗi:** Bắt lỗi hệ thống tập tin toàn diện (`errno`, `strerror`), cho phép chương trình thông báo lỗi chuẩn xác ra `stderr` và tiếp tục xử lý các đối số hợp lệ kế tiếp mà không bị crash đột ngột.

### Các khái niệm cốt lõi UNIX / POSIX áp dụng trong dự án
* **Duyệt hệ thống tập tin & I/O thư mục:** Sử dụng `opendir()`, `readdir()`, `closedir()`, lọc mục ẩn và mục đặc biệt (`.` và `..`).
* **Trích xuất siêu dữ liệu tập tin (Metadata) & Inode:** `lstat()`, `stat()`, `readlink()`, phân loại tập tin (regular file, directory, symlink, block/character device, FIFO, socket), thao tác trên mặt nạ quyền (`mode_t`, `st_mode`), cờ đặc biệt (`setuid`, `setgid`, `sticky bit`), và số hiệu Inode.
* **Tra cứu cơ sở dữ liệu Người dùng & Nhóm:** `getpwuid()`, `getgrgid()` để phân giải UID/GID thành tên tài khoản/nhóm tương ứng.
* **Nhận diện thiết bị ngoại vi & Dòng dữ liệu (Terminal/Stream):** `isatty(STDOUT_FILENO)` nhằm thiết lập giá trị mặc định theo ngữ cảnh (chế độ `-q` thay thế ký tự không in được khi xuất ra terminal so với `-w` khi xuất ra file/pipe; in tổng dung lượng block trước danh sách đối với `-s`).
* **Biến môi trường:** Đọc và áp dụng biến môi trường `BLOCKSIZE` để điều chỉnh đơn vị tính block.
* **Thuật toán sắp xếp đa tiêu chí:** Ứng dụng hàm chuẩn `qsort()` với thuật toán so sánh tùy biến (theo tên, kích thước, thời gian, đảo ngược, không sắp xếp), hỗ trợ phá vỡ thế hòa (tie-breaking) tất định theo tên từ điển.
* **Quản trị bộ nhớ an toàn:** Cấp phát động (`malloc`, `calloc`, `realloc`, `strdup`) có đối ứng giải phóng (`free`) triệt để, đã qua thẩm định của AddressSanitizer (0 memory leak, 0 buffer overflow).

---

## 3. Hướng dẫn biên dịch và chạy (Quick Start)

### Bước 1: Clone repo từ GitHub về máy và chuyển vào thư mục dự án
Mở cửa sổ dòng lệnh Terminal và thực hiện:
```bash
git clone https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm.git
cd LeHoaiDuc_24IT339_midterm
```

### Bước 2: Biên dịch chương trình bằng lệnh `make`
Dự án được trang bị sẵn một `Makefile`. Để biên dịch, bạn chỉ cần chạy:
```bash
make
```

**Giải thích cơ chế biên dịch:**
* Trình biên dịch sử dụng: `CC = cc` (tương thích cả GCC và Clang).
* Cờ biên dịch nghiêm ngặt:
  ```makefile
  CFLAGS = -Wall -Wextra -Werror -Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_NETBSD_SOURCE
  ```
  - `-Wall -Wextra`: Bật toàn bộ các cảnh báo biên dịch cơ bản và mở rộng.
  - `-Werror`: Chuyển tất cả cảnh báo thành lỗi biên dịch, đảm bảo mã nguồn sạch sẽ tuyệt đối.
  - `-Iinclude`: Khai báo đường dẫn chứa các file header `.h`.
  - `-D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE`: Kích hoạt chuẩn POSIX.1-2008 và giao diện hệ thống UNIX chuẩn (cho phép dùng `lstat`, `major`, `minor`, `st_blocks`,...).
  - `-D_NETBSD_SOURCE`: Tối ưu hóa tính tương thích trên hệ điều hành NetBSD và các hệ thống dòng BSD.
* Quá trình biên dịch sẽ tạo riêng từng file đối tượng `.o` trong thư mục `src/` (`main.o`, `options.o`, `file_info.o`, `display.o`, `sort.o`, `traverse.o`) và liên kết chúng thành file thực thi duy nhất là `./ls`.

### Bước 3: Chạy thử các câu lệnh cơ bản và nâng cao

Chương trình được chạy bằng lệnh `./ls` (tránh nhầm lẫn với lệnh `/bin/ls` mặc định của hệ thống).

#### 1. Liệt kê cơ bản (mỗi file một dòng):
```bash
./ls
```
*Kết quả mẫu:*
```text
Makefile
README.md
include
ls
src
tests
```

#### 2. Liệt kê chi tiết dạng danh sách dài (Long format):
```bash
./ls -l
```
*Kết quả mẫu:*
```text
total 112
-rw-r--r-- 1 hduc  hduc    527 Oct  7 21:26 Makefile
-rw-rw-r-- 1 hduc  hduc  17006 Oct  7 21:33 README.md
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:25 include
-rwxrwxr-x 1 hduc  hduc  26960 Oct  7 21:26 ls
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:26 src
drwxrwxr-x 2 hduc  hduc   4096 Oct  7 21:29 tests
```

#### 3. Hiển thị dung lượng dạng con người đọc được (`-h`):
```bash
./ls -lh
```
*Kết quả mẫu:*
```text
total 56K
-rw-r--r-- 1 hduc  hduc  527B Oct  7 21:26 Makefile
-rw-rw-r-- 1 hduc  hduc   17K Oct  7 21:33 README.md
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:25 include
-rwxrwxr-x 1 hduc  hduc   26K Oct  7 21:26 ls
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:26 src
drwxrwxr-x 2 hduc  hduc  4.0K Oct  7 21:29 tests
```

#### 4. Hiển thị file ẩn (`-a` và `-A`):
```bash
# Hiển thị tất cả file ẩn, bao gồm cả '.' và '..'
./ls -a

# Hiển thị tất cả file ẩn, nhưng loại bỏ '.' và '..'
./ls -A
```

#### 5. Hiển thị ký hiệu nhận diện định dạng file (`-F`):
```bash
./ls -F
```
*Thư mục được gắn `/`, file thực thi gắn `*`, liên kết mềm gắn `@`,...*

#### 6. Hiển thị số hiệu Inode (`-i`) và khối đĩa (`-s`):
```bash
./ls -lis
```

#### 7. Sắp xếp theo kích thước và thời gian:
```bash
# Sắp xếp theo dung lượng giảm dần (file lớn nhất trước)
./ls -lS

# Sắp xếp theo dung lượng tăng dần (đảo ngược thứ tự)
./ls -lSr

# Sắp xếp theo thời gian chỉnh sửa mới nhất trước
./ls -lt

# Không sắp xếp (theo thứ tự vật lý thư mục)
./ls -f
```

#### 8. Xem thông tin chính thư mục mà không duyệt nội dung (`-d`):
```bash
./ls -ld src include
```

#### 9. Duyệt đệ quy toàn bộ thư mục con (`-R`):
```bash
./ls -R include
```

#### 10. Truyền nhiều đối số hỗn hợp (file và thư mục):
```bash
./ls Makefile include src
```
*(Chương trình sẽ tự động hiển thị các file trước, sau đó sắp xếp và hiển thị từng thư mục).*

### Bước 4: Chạy bộ kiểm thử tự động
Dự án tích hợp kịch bản kiểm thử toàn diện `tests/run_tests.sh`. Để kích hoạt kiểm thử:
```bash
make test
```
*Tất cả 15 bài test kiểm tra biên sẽ chạy tự động và báo `All tests passed successfully!`.*

### Bước 5: Dọn dẹp file build
Khi muốn xóa toàn bộ các file `.o` và file nhị phân `./ls` để đưa mã nguồn về trạng thái nguyên bản:
```bash
make clean
```

### Hướng dẫn dành riêng cho BSD / Máy ảo NetBSD (VirtualBox / QEMU)

Nếu bạn kiểm thử chương trình trực tiếp trên môi trường **NetBSD** (hoặc FreeBSD) cài đặt trên máy thật hoặc máy ảo (VirtualBox / QEMU / VMware), các bước thực hiện cực kỳ đơn giản như sau:

#### 1. Đưa mã nguồn vào máy ảo NetBSD
Tùy thuộc vào cấu hình mạng của máy ảo, bạn có thể chọn 1 trong 3 cách sau:

* **Cách 1: Sử dụng lệnh `ftp(1)` có sẵn của NetBSD (Không cần cài thêm gói gì):**
  NetBSD tích hợp sẵn công cụ dòng lệnh `ftp` hỗ trợ tải qua giao thức HTTP/HTTPS rất tiện lợi:
  ```sh
  ftp https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm/archive/refs/heads/main.tar.gz
  tar -xzf main.tar.gz
  cd LeHoaiDuc_24IT339_midterm-main
  ```

* **Cách 2: Clone trực tiếp bằng `git` (Nếu máy ảo đã cài git):**
  ```sh
  # Cài đặt git qua pkgin nếu chưa có:
  pkgin update && pkgin install git

  # Clone repo và chuyển vào thư mục:
  git clone https://github.com/HOAIDUC-L/LeHoaiDuc_24IT339_midterm.git
  cd LeHoaiDuc_24IT339_midterm
  ```

* **Cách 3: Sao chép từ máy Host sang máy ảo NetBSD qua SSH / SCP:**
  Nếu máy ảo bật dịch vụ SSH (ví dụ dùng NAT port forwarding sang cổng 2222 của Host):
  ```sh
  # Chạy lệnh này trên máy Host (Linux / macOS / Windows Terminal):
  scp -P 2222 -r LeHoaiDuc_24IT339_midterm root@127.0.0.1:/root/
  ```

#### 2. Biên dịch trên NetBSD
Hệ điều hành NetBSD có sẵn trình biên dịch C (`cc` / `gcc`) và tiện ích `make` trong hệ thống cơ sở (Base System Developer Tools). Bạn không cần cài đặt thêm công cụ biên dịch nào:
```sh
make clean && make
```
*Lưu ý:* `Makefile` của dự án đã bổ sung cờ `-D_NETBSD_SOURCE` và sử dụng cú pháp chuẩn POSIX nên tương thích 100% với công cụ `make` mặc định của NetBSD (`bmake`) cũng như `gmake`.

#### 3. Chạy và kiểm thử trên NetBSD
* Chạy thử các câu lệnh:
  ```sh
  ./ls -la
  ./ls -lh
  ./ls -F
  ```
* Chạy bộ kiểm thử tự động 15 bài test:
  ```sh
  make test
  ```
  *(Kịch bản kiểm thử `tests/run_tests.sh` được lập trình hoàn toàn bằng chuẩn POSIX `/bin/sh`, tương thích trực tiếp với shell mặc định của NetBSD mà không bắt buộc phải cài đặt bash).*

---

## 4. Danh sách 19 cờ (Options) và Quy tắc ưu tiên theo NetBSD 10.1

Toàn bộ 19 cờ xuất hiện trong bản đặc tả manual page `ls [ -AacdFfhiklnqRrSstuw] [file ...]` đều được hiện thực hóa đầy đủ:

| Cờ (Flag) | Tên gọi & Ý nghĩa | Chi tiết cài đặt & Hành vi kỹ thuật |
| :---: | :--- | :--- |
| `-A` | List Almost All (Liệt kê gần hết) | Hiển thị tất cả các file ẩn ngoại trừ `.` và `..`. Tự động bật mặc định đối với tài khoản root (`geteuid() == 0`). |
| `-a` | List All (Liệt kê toàn bộ) | Hiển thị tất cả các mục thư mục bắt đầu bằng dấu chấm (`.`), bao gồm cả `.` và `..`. |
| `-c` | Status Change Time (Thời gian trạng thái) | Sử dụng thời điểm trạng thái thay đổi (`st_ctime`) thay vì thời điểm chỉnh sửa (`st_mtime`) khi sắp xếp (`-t`) hoặc in (`-l`). Ghi đè và bị ghi đè bởi `-u`. |
| `-d` | Directory as File (Thư mục như file) | Coi các thư mục là file thông thường (không duyệt đệ quy vào trong) và không giải tham chiếu liên kết mềm trong danh sách đối số. Ghi đè và bị ghi đè bởi `-R`. |
| `-F` | File Classification (Ký hiệu phân loại) | Thêm dấu gạch chéo `/` sau thư mục, dấu sao `*` sau file thực thi, `@` sau liên kết mềm, `%` sau whiteout, `=` sau socket, và `\|` sau FIFO. |
| `-f` | No Sort (Không sắp xếp) | Vô hiệu hóa toàn bộ việc sắp xếp; các mục xuất hiện theo đúng thứ tự đọc từ đĩa. |
| `-h` | Human-Readable Sizes (Dung lượng dễ đọc) | Điều chỉnh định dạng của cờ `-s` và `-l` để hiển thị kích thước theo các đơn vị chuẩn (B, K, M, G, T, P, E) theo chuẩn `humanize_number(3)`. Ghi đè cờ `-k`. |
| `-i` | Inode Number (Số hiệu Inode) | In số hiệu Inode (`st_ino`) của từng file trước tên hoặc các thông số khác. |
| `-k` | Kilobytes Block (Đơn vị Kilobyte) | Điều chỉnh cờ `-s` để báo cáo kích thước khối tính theo đơn vị 1024 bytes (1 KB). Cờ nào xuất hiện sau cùng giữa `-k` và `-h` sẽ có hiệu lực. |
| `-l` | Long Listing (Định dạng dài) | Hiển thị đầy đủ chế độ file, số liên kết cứng, chủ sở hữu, nhóm, kích thước (hoặc major/minor với device), thời gian và đường dẫn (kèm `-> target` nếu là liên kết mềm). Ghi đè và bị ghi đè bởi `-n`. |
| `-n` | Numeric IDs (ID dạng số) | Tương tự `-l`, nhưng hiển thị UID và GID dưới dạng số nguyên thay vì giải mã sang tên người dùng/nhóm. Ghi đè và bị ghi đè bởi `-l`. |
| `-q` | Non-Printable as '?' (Ký tự lạ thành '?') | Ép buộc thay thế các ký tự không in được trong tên file thành dấu hỏi `?`. Mặc định được bật khi đầu ra stdout kết nối với Terminal. Ghi đè và bị ghi đè bởi `-w`. |
| `-R` | Recursive (Duyệt đệ quy) | Duyệt đệ quy vào tất cả các thư mục con gặp phải (theo chiều sâu). Tuyệt đối không theo liên kết mềm trỏ tới thư mục để tránh vòng lặp vô tận. Ghi đè và bị ghi đè bởi `-d`. |
| `-r` | Reverse Sort (Đảo ngược sắp xếp) | Đảo ngược thứ tự sắp xếp (ví dụ: thứ tự bảng chữ cái ngược, file nhỏ nhất lên trước, hoặc file cũ nhất lên trước). |
| `-S` | Sort by Size (Sắp xếp theo dung lượng) | Sắp xếp theo dung lượng file giảm dần (`st_size`). Giải quyết các trường hợp bằng nhau bằng tên bảng chữ cái. |
| `-s` | Display Blocks (Hiển thị khối đĩa) | Hiển thị số khối đĩa thực tế file chiếm dụng theo đơn vị 512 bytes (hoặc theo `BLOCKSIZE` / 1024 nếu có `-k`). In tổng số block trước danh sách khi đầu ra là Terminal. |
| `-t` | Sort by Time (Sắp xếp theo thời gian) | Sắp xếp theo thời gian (mới nhất trước). Kết hợp với `-c` hoặc `-u` để chọn loại thời gian. Bẻ thế hòa bằng tên bảng chữ cái. |
| `-u` | Access Time (Thời gian truy cập) | Sử dụng thời điểm truy nhập cuối (`st_atime`) thay vì `st_mtime` cho việc sắp xếp (`-t`) hoặc hiển thị (`-l`). Ghi đè và bị ghi đè bởi `-c`. |
| `-w` | Raw Non-Printable (In thô ký tự lạ) | Ép buộc in nguyên bản các ký tự không in được. Mặc định bật khi đầu ra không phải Terminal (file, pipe). Ghi đè và bị ghi đè bởi `-q`. |

### Quy tắc ghi đè & Thứ tự ưu tiên (Option Interactions & Precedence)
1. **Ký tự không in được:** `-w` và `-q` ghi đè lẫn nhau; cờ nào xuất hiện cuối cùng trên dòng lệnh sẽ quyết định cách xuất.
2. **Định dạng hiển thị UID/GID:** `-l` và `-n` ghi đè lẫn nhau; cờ chỉ định sau cùng sẽ quyết định hiển thị tên hay số.
3. **Mốc thời gian sử dụng:** `-c` và `-u` ghi đè lẫn nhau; cờ sau cùng quyết định lấy `st_ctime` hay `st_atime` (mặc định không có là `st_mtime`).
4. **Hành vi duyệt thư mục:** `-R` và `-d` ghi đè lẫn nhau; nếu `-d` sau cùng thì không đệ quy, nếu `-R` sau cùng thì bật đệ quy.
5. **Thang đo kích thước khối:** Cờ nằm bên phải nhất giữa `-k` và `-h` sẽ ghi đè cờ nằm trước.
6. **Thứ bậc đơn vị block cho cờ `-s`:**
   - Nếu có `-h`: Tự động co giãn theo dung lượng dễ đọc (B, K, M, G).
   - Nếu có `-k`: Khối cố định 1024 bytes (1 KB).
   - Nếu không có `-h` và `-k`: Kiểm tra biến môi trường `BLOCKSIZE`. Nếu được đặt và hợp lệ thì dùng đơn vị này.
   - Mặc định: Đơn vị chuẩn UNIX 512 bytes.

---

## 5. Cấu trúc kiến trúc mô-đun (Modular Architecture)

Dự án được phân tách nghiêm ngặt thành các mô-đun độc lập, mỗi mô-đun đảm nhận một nhóm chức năng rõ ràng, mã nguồn đặt tại `src/` và các tệp giao diện đặt tại `include/`:

```text
LeHoaiDuc_24IT339_midterm/
├── Makefile                # Kịch bản biên dịch tiêu chuẩn, kiểm thử tự động và dọn dẹp
├── README.md               # Bản báo cáo kỹ thuật & tài liệu hướng dẫn hoàn chỉnh
├── .gitignore              # Loại trừ file nhị phân, file đối tượng .o, IDE và tệp tạm
│
├── include/
│   ├── options.h           # Định nghĩa struct Options, enum tiêu chí và prototype hàm phân tích cờ
│   ├── file_info.h         # Cấu trúc FileInfo, FileInfoList và các hàm thu thập siêu dữ liệu
│   ├── display.h           # Các hàm định dạng chuỗi quyền, kích thước dễ đọc, thời gian và bảng biểu
│   ├── sort.h              # Thuật toán sắp xếp đa khóa, tie-breaking và đảo ngược danh sách
│   └── traverse.h          # Điều phối đối số, đọc thư mục opendir/readdir và đệ quy -R
│
├── src/
│   ├── main.c              # Điểm khởi nhập (entry point), điều phối và trả về exit status
│   ├── options.c           # Phân tích cú pháp cờ lệnh bằng getopt() và giải quyết ghi đè
│   ├── file_info.c         # Gọi lstat/stat, đọc readlink động, tra cứu passwd/group và giải phóng bộ nhớ
│   ├── display.c           # Xử lý căn lề cột linh hoạt, in total, định dạng ngày tháng 6 tháng NetBSD
│   ├── sort.c              # Cài đặt hàm qsort so sánh theo tên, kích thước, thời gian, nosort
│   └── traverse.c          # Quản lý hàng đợi đối số, mở thư mục, lọc mục ẩn và duyệt đệ quy
│
└── tests/
    └── run_tests.sh        # Kịch bản kiểm thử tự động 15 bộ kịch bản kiểm thử biên
```

### Chi tiết nhiệm vụ từng mô-đun

1. **`main.c`:**
   * Là điểm vào của chương trình (`int main(int argc, char *argv[])`).
   * Khởi tạo cấu hình mặc định bằng `options_init()`.
   * Chuyển các tham số dòng lệnh cho `options_parse()`.
   * Giao quyền xử lý cho `traverse_operands()` và chuyển đổi mã lỗi thành `EXIT_SUCCESS` (0) hoặc `EXIT_FAILURE` (>0).

2. **`options.c / options.h`:**
   * Khai báo cấu trúc dữ liệu `Options` quản lý toàn bộ trạng thái cấu hình.
   * Xác định các giá trị mặc định lúc chạy: kiểm tra Terminal bằng `isatty(STDOUT_FILENO)`, kiểm tra quyền Root bằng `geteuid() == 0` (tự động bật `-A`), và đọc biến môi trường `BLOCKSIZE`.
   * Duyệt qua chuỗi tùy chọn `"AacdFfhiklnqRrSstuw"` thông qua hàm `getopt()` và thực thi logic tiền đề/ghi đè.

3. **`file_info.c / file_info.h`:**
   * Cung cấp cấu trúc `FileInfo` chứa: tên hiển thị, đường dẫn đầy đủ, `struct stat`, tên chủ sở hữu, tên nhóm, đường dẫn đích của symlink, các cờ kiểm tra định dạng.
   * Định nghĩa mảng động `FileInfoList` với khả năng tự động tăng gấp đôi dung lượng khi đầy.
   * Xử lý chính xác sự khác biệt giữa `stat()` và `lstat()`:
     - Đối với các phần tử bên trong thư mục: luôn dùng `lstat()`.
     - Đối với đối số truyền từ dòng lệnh: nếu không có `-d`, `-l`, `-F` thì dùng `stat()` trước để giải tham chiếu symlink trỏ tới thư mục; nếu có `-d`, `-l`, `-F` thì dùng `lstat()`.
   * Đọc đường dẫn đích của symbolic link bằng `readlink()` động (tự động mở rộng buffer, không giả định kích thước `PATH_MAX` cố định, luôn đảm bảo kết thúc bằng ký tự null `\0`).
   * Truy vấn tên người dùng/nhóm bằng `getpwuid()` / `getgrgid()`, nếu không tìm thấy sẽ chuyển sang biểu diễn dạng số nguyên.

4. **`sort.c / sort.h`:**
   * Sử dụng thuật toán `qsort()` chuẩn C.
   * Xử lý so sánh theo 3 tiêu chí chính: Tên bảng chữ cái (`strcmp`), Kích thước (`st_size`), và Thời gian (hỗ trợ độ chính xác nano giây qua `st_mtim.tv_nsec` / `st_ctim.tv_nsec` / `st_atim.tv_nsec`).
   * Cơ chế Tie-breaking tất định: Khi hai file có kích thước hoặc mốc thời gian bằng nhau, luôn dùng tên theo thứ tự từ điển làm tiêu chí phụ.
   * Hỗ trợ đảo ngược (`-r`) và bỏ qua sắp xếp (`-f`).

5. **`display.c / display.h`:**
   * Tạo chuỗi 10 ký tự quyền hạn chuẩn UNIX từ `st_mode`: chữ cái đầu thể hiện loại file (`d`, `c`, `b`, `l`, `s`, `p`, `-`), cùng đầy đủ `rwx`, `setuid` (`s`/`S`), `setgid` (`s`/`S`), `sticky bit` (`t`/`T`).
   * Cài đặt thuật toán `display_humanize_number()` chuẩn NetBSD để định dạng dung lượng dạng `123B`, `1.2K`, `15K`, `2.5M`, `10G`.
   * Cài đặt quy tắc thời gian 6 tháng của NetBSD: Các file có mốc thời gian trong vòng 6 tháng gần nhất sẽ in định dạng `Mmm dd HH:MM` (ví dụ `Oct  7 21:26`), các file cũ hơn 6 tháng hoặc trong tương lai sẽ in dạng `Mmm dd  YYYY` (ví dụ `Oct  7  2024`).
   * Tính toán độ rộng động lớn nhất của từng cột (Inode, Block, Link, Owner, Group, Size) trong danh sách để căn lề thẳng hàng tuyệt đối.
   * In dòng tổng khối đĩa (`total <số>`) trước nội dung thư mục khi ở chế độ `-l` hoặc khi có `-s` chạy trên Terminal.
   * Hiển thị số `major, minor` thay cho kích thước đối với các file thiết bị (`character` hoặc `block` special device).

6. **`traverse.c / traverse.h`:**
   * Tách danh sách đối số ban đầu thành hai danh sách con riêng biệt: `non_dir_list` (tập tin không phải thư mục) và `dir_list` (các thư mục).
   * Sắp xếp độc lập cả hai danh sách theo tiêu chí người dùng yêu cầu, sau đó in các tệp không phải thư mục lên trước.
   * Duyệt thư mục bằng `opendir()`, `readdir()`, `closedir()`. Đặt lại `errno = 0` ngay trước mỗi lệnh gọi `readdir()` để tránh hiểu lầm các lỗi hệ thống phát sinh bên trong thân vòng lặp thành lỗi kết thúc thư mục.
   * Triển khai đệ quy `-R` theo chiều sâu, đồng thời bỏ qua các symbolic link trỏ đến thư mục để phòng chống lặp vô hạn.

---

## 6. Các ca kiểm thử biên (Edge Cases) & An toàn bộ nhớ

Dự án đã được kiểm tra nghiêm ngặt với 15 bộ kịch bản kiểm thử trong `tests/run_tests.sh`:

1. **Thư mục trống (Empty Directory):**
   * Chạy `./ls empty_dir`: không in gì và thoát mã 0.
   * Chạy `./ls -l empty_dir`: in chuẩn xác dòng `total 0` và thoát mã 0.
2. **Liên kết mềm bị gãy (Broken Symbolic Links):**
   * Tạo symlink trỏ đến file không tồn tại: `./ls -l` vẫn đọc được liên kết mềm qua `lstat()`, in ra đúng dạng `broken_symlink -> nonexistent_target` mà không làm crash chương trình.
3. **Tên file có dấu cách và ký tự đặc biệt:**
   * Tên file chứa khoảng trắng (`file with spaces.txt`): Xử lý hiển thị chuẩn xác, không bị ngắt từ.
   * Tên file chứa ký tự điều khiển/không in được (ASCII byte `0x01`):
     - Khi chạy với `-q` (mặc định terminal): Ký tự lạ được thay thế bằng `?` (`file?nonprint.txt`).
     - Khi chạy với `-w`: In thô nguyên bản byte điều khiển.
4. **Tập tin đặc biệt (FIFO, Socket, Device Nodes):**
   * Kiểm tra named pipe (FIFO): Cờ `-F` gắn dấu gạch đứng `|`, cờ `-l` nhận diện đúng loại file `p`.
   * Kiểm tra thiết bị `/dev/null`: Cờ `-l` in loại `c` và thay trường kích thước bằng số định danh `1, 3` (major, minor).
5. **Nhiều đối số hỗn hợp:**
   * Chạy `./ls file.txt dir1 dir2`: File thông thường hiển thị trước, sau đó là tiêu đề `dir1:` cùng nội dung, tiếp đến là tiêu đề `dir2:` cùng nội dung, ngăn cách bằng dòng trắng rõ ràng.
6. **Xử lý lỗi truy cập & Không tồn tại:**
   * Thử nghiệm với file không tồn tại: Báo lỗi chuẩn mực ra luồng `stderr`:
     ```text
     ls: /tmp/nonexistent: No such file or directory
     ```
   * Giá trị trả về của chương trình tự động chuyển sang mã `1` (`> 0`) đúng yêu cầu đặc tả.
7. **Kiểm tra an toàn bộ nhớ (Memory Safety):**
   * Chương trình được biên dịch và chạy qua toàn bộ test suite với **AddressSanitizer** và **UndefinedBehaviorSanitizer**:
     ```bash
     cc -fsanitize=address,undefined src/*.c -Iinclude -o ls
     ./tests/run_tests.sh
     ```
   * **Kết quả:** Phát hiện **0** rò rỉ bộ nhớ (memory leaks), **0** lỗi tràn bộ đệm (buffer overflow), **0** lỗi sử dụng con trỏ sau khi giải phóng (use-after-free).

---

## 7. Các giới hạn đã biết (Known Limitations)

Để bảo đảm tính tuân thủ nghiêm ngặt theo đặc tả đề bài và tài liệu NetBSD 10.1 được giao, các tính năng sau đây được **chủ động không cài đặt**:
* **Không hỗ trợ cờ ngoài phạm vi:** Chỉ hỗ trợ chính xác 19 cờ trong synopsis. Các cờ phổ biến trên Linux GNU như `--color`, `-C` (in nhiều cột), `-m` (danh sách phân tách bằng dấu phẩy), `-x` (sắp xếp theo dòng ngang), `-1` (bắt buộc 1 cột) đều không được thêm vào.
* **Mặc định một mục trên một dòng:** Đúng theo quy định của manual page NetBSD: *"By default, ls lists one entry per line to standard output."*
* **Không hỗ trợ màu sắc:** Không nhúng mã ANSI Escape Color vào tên file để đảm bảo tính thuần khiết và tính tương thích trên mọi môi trường POSIX tối giản.
* **Không hỗ trợ ký hiệu mở rộng ACL (+):** Các cờ mở rộng Access Control List trên BSD/Solaris đòi hỏi các thư viện không thuộc chuẩn POSIX.2 nên không được đưa vào.

---

## 8. Kết luận

Dự án cài đặt thành công công cụ `ls(1)` đáp ứng các yêu cầu:
- Đúng chuẩn C99 / POSIX, kiến trúc mô-đun rõ ràng.
- Đầy đủ 19 cờ và mọi quy tắc ưu tiên theo tài liệu gốc.
- Tài liệu báo cáo chi tiết, mạch lạc .
