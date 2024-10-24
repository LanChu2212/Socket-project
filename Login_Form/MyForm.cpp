
#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Sử dụng đúng tên namespace và lớp MyForm
    LoginForm::MyForm form;
    Application::Run(% form);
    //StartServer(); // Gọi hàm khởi động server khi cần thiết
    return 0;


}