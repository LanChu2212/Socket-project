#include "ServerForm.h"
#pragma once

namespace LoginForm {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    public ref class MyForm : public System::Windows::Forms::Form
    {
    public:
        MyForm(void)
        {
            InitializeComponent();
        }

    protected:
        ~MyForm()
        {
            if (components)
            {
                delete components;
            }
        }

    private:
        System::Windows::Forms::Label^ label1;
        System::Windows::Forms::Label^ label2;
        System::Windows::Forms::Label^ label3;
        System::Windows::Forms::TextBox^ textBox1;
        System::Windows::Forms::CheckBox^ checkBox1;
        System::Windows::Forms::TextBox^ textBox2;
        System::Windows::Forms::Label^ label4;
        System::Windows::Forms::Label^ label5;
        System::Windows::Forms::Button^ button1;
        System::Windows::Forms::Button^ button2;
        System::Windows::Forms::Button^ buttonShowPassword;  // New button to toggle password visibility

        System::ComponentModel::Container^ components;
       

#pragma region Windows Form Designer generated code
        void InitializeComponent(void)
        {
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
            this->textBox2 = (gcnew System::Windows::Forms::TextBox());
            this->label4 = (gcnew System::Windows::Forms::Label());
            this->label5 = (gcnew System::Windows::Forms::Label());
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->button2 = (gcnew System::Windows::Forms::Button());
            this->buttonShowPassword = (gcnew System::Windows::Forms::Button());
            this->SuspendLayout();
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->BackColor = System::Drawing::Color::Transparent;
            this->label1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 24, System::Drawing::FontStyle::Bold));
            this->label1->Location = System::Drawing::Point(291, 70);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(153, 54);
            this->label1->TabIndex = 0;
            this->label1->Text = L"Sign In";
            this->label1->Click += gcnew System::EventHandler(this, &MyForm::label1_Click);
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->BackColor = System::Drawing::Color::Transparent;
            this->label2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->label2->Location = System::Drawing::Point(259, 138);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(87, 23);
            this->label2->TabIndex = 1;
            this->label2->Text = L"Username";
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->BackColor = System::Drawing::Color::Transparent;
            this->label3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->label3->Location = System::Drawing::Point(259, 203);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(80, 23);
            this->label3->TabIndex = 2;
            this->label3->Text = L"Password";
            // 
            // textBox1
            // 
            this->textBox1->BackColor = System::Drawing::Color::White;
            this->textBox1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->textBox1->Location = System::Drawing::Point(263, 161);
            this->textBox1->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->textBox1->Name = L"textBox1";
            this->textBox1->Size = System::Drawing::Size(240, 30);
            this->textBox1->TabIndex = 3;
            this->textBox1->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged);
            // 
            // checkBox1
            // 
            this->checkBox1->AutoSize = true;
            this->checkBox1->BackColor = System::Drawing::Color::Transparent;
            this->checkBox1->Location = System::Drawing::Point(263, 267);
            this->checkBox1->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->checkBox1->Name = L"checkBox1";
            this->checkBox1->Size = System::Drawing::Size(85, 20);
            this->checkBox1->TabIndex = 4;
            this->checkBox1->Text = L"I agree to";
            this->checkBox1->UseVisualStyleBackColor = false;
            // 
            // textBox2
            // 
            this->textBox2->BackColor = System::Drawing::Color::White;
            this->textBox2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->textBox2->Location = System::Drawing::Point(263, 226);
            this->textBox2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->textBox2->Name = L"textBox2";
            this->textBox2->PasswordChar = '*';
            this->textBox2->Size = System::Drawing::Size(240, 30);
            this->textBox2->TabIndex = 5;
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->label4->Location = System::Drawing::Point(257, 228);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(0, 23);
            this->label4->TabIndex = 6;
            // 
            // label5
            // 
            this->label5->AutoSize = true;
            this->label5->BackColor = System::Drawing::Color::Transparent;
            this->label5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->label5->ForeColor = System::Drawing::Color::DodgerBlue;
            this->label5->Location = System::Drawing::Point(357, 265);
            this->label5->Name = L"label5";
            this->label5->Size = System::Drawing::Size(136, 23);
            this->label5->TabIndex = 7;
            this->label5->Text = L"Terms and Policy";
            // 
            // button1
            // 
            this->button1->BackColor = System::Drawing::Color::LightCyan;
            this->button1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
            this->button1->ForeColor = System::Drawing::Color::Red;
            this->button1->Location = System::Drawing::Point(263, 294);
            this->button1->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(85, 37);
            this->button1->TabIndex = 8;
            this->button1->Text = L"Sign In";
            this->button1->UseVisualStyleBackColor = false;
            this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
            // 
            // button2
            // 
            this->button2->BackColor = System::Drawing::Color::Transparent;
            this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->button2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->button2->Location = System::Drawing::Point(363, 294);
            this->button2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(93, 37);
            this->button2->TabIndex = 9;
            this->button2->Text = L"Create New Account";
            this->button2->UseVisualStyleBackColor = false;
            this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
            // 
            // buttonShowPassword
            // 
            this->buttonShowPassword->BackColor = System::Drawing::Color::Transparent;
            this->buttonShowPassword->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
            this->buttonShowPassword->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
            this->buttonShowPassword->Location = System::Drawing::Point(509, 222);
            this->buttonShowPassword->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->buttonShowPassword->Name = L"buttonShowPassword";
            this->buttonShowPassword->Size = System::Drawing::Size(47, 41);
            this->buttonShowPassword->TabIndex = 10;
            this->buttonShowPassword->Text = L"👁";
            this->buttonShowPassword->UseVisualStyleBackColor = false;
            this->buttonShowPassword->Click += gcnew System::EventHandler(this, &MyForm::buttonShowPassword_Click);
            // 
            // MyForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
            this->ClientSize = System::Drawing::Size(800, 450);
            this->Controls->Add(this->buttonShowPassword);
            this->Controls->Add(this->button2);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->label5);
            this->Controls->Add(this->label4);
            this->Controls->Add(this->textBox2);
            this->Controls->Add(this->checkBox1);
            this->Controls->Add(this->textBox1);
            this->Controls->Add(this->label3);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->label1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->MaximizeBox = false;
            this->Name = L"MyForm";
            this->Text = L"LoginForm";
            this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
            this->ResumeLayout(false);
            this->PerformLayout();
            // Add MouseEnter, MouseLeave, MouseDown, MouseUp events
            this->button1->MouseEnter += gcnew System::EventHandler(this, &MyForm::button1_MouseEnter);
            this->button1->MouseLeave += gcnew System::EventHandler(this, &MyForm::button1_MouseLeave);
            this->button1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::button1_MouseDown);
            this->button1->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::button1_MouseUp);

            this->button2->MouseEnter += gcnew System::EventHandler(this, &MyForm::button2_MouseEnter);
            this->button2->MouseLeave += gcnew System::EventHandler(this, &MyForm::button2_MouseLeave);
            this->button2->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::button2_MouseDown);
            this->button2->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::button2_MouseUp);

            this->textBox1->Enter += gcnew System::EventHandler(this, &MyForm::textBox1_Enter);
            this->textBox1->Leave += gcnew System::EventHandler(this, &MyForm::textBox1_Leave);
            this->textBox2->Enter += gcnew System::EventHandler(this, &MyForm::textBox2_Enter);
            this->textBox2->Leave += gcnew System::EventHandler(this, &MyForm::textBox2_Leave);

        }
#pragma endregion

    private: System::Void label1_Click(System::Object^ sender, System::EventArgs^ e) {
    }
    private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
    }
    private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
        if (String::IsNullOrEmpty(textBox1->Text) || String::IsNullOrEmpty(textBox2->Text)) {
            MessageBox::Show(L"Please fill in both fields!", L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
        else if (!checkBox1->Checked) {
            MessageBox::Show(L"You must agree to the terms!", L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
        else {
            // Nếu đăng nhập thành công, hiển thị ServerForm
            MessageBox::Show(L"Sign In Successful!");

            // Tạo và hiển thị ServerForm
            LoginForm::ServerForm^ serverForm = gcnew LoginForm::ServerForm();
            serverForm->Show();


            // Đóng LoginForm (hoặc ẩn LoginForm)
            this->Hide(); // Ẩn LoginForm
        }
    }
    private: System::Void buttonShowPassword_Click(System::Object^ sender, System::EventArgs^ e) {
        System::Threading::Thread::Sleep(50);  // Add a small delay for smoother effect
        if (textBox2->PasswordChar == '*') {
            textBox2->PasswordChar = '\0';
            buttonShowPassword->Text = L"👁";  // Show icon
        }
        else {
            textBox2->PasswordChar = '*';
            buttonShowPassword->Text = L"🙈";  // Hide icon
        }
    }

    private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
    }
    private: System::Void MyForm_Load(System::Object^ sender, System::EventArgs^ e) {
        this->Opacity = 0; // Start fully transparent
        for (double i = 0; i <= 1; i += 0.05) {
            this->Opacity = i; // Gradually increase opacity
            System::Threading::Thread::Sleep(50); // Pause for a brief moment
        }
    }

private: System::Void button1_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
    button1->BackColor = System::Drawing::Color::DodgerBlue;
}

private: System::Void button1_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
    button1->BackColor = System::Drawing::Color::LightCyan;
}

private: System::Void button1_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
    button1->BackColor = System::Drawing::Color::RoyalBlue; // Darker color on click
}

private: System::Void button1_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
    button1->BackColor = System::Drawing::Color::LightCyan; // Reset color when released
}

private: System::Void button2_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
    button2->BackColor = System::Drawing::Color::DodgerBlue;
}

private: System::Void button2_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
    button2->BackColor = System::Drawing::Color::Transparent;
}

private: System::Void button2_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
    button2->BackColor = System::Drawing::Color::RoyalBlue; // Darker color on click
}

private: System::Void button2_MouseUp(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
    button2->BackColor = System::Drawing::Color::Transparent; // Reset color when released
}

private: System::Void textBox1_Enter(System::Object^ sender, System::EventArgs^ e) {
    textBox1->BackColor = System::Drawing::Color::LightYellow; // Highlight on focus
}

private: System::Void textBox1_Leave(System::Object^ sender, System::EventArgs^ e) {
    textBox1->BackColor = System::Drawing::Color::White; // Reset color when focus is lost
}

private: System::Void textBox2_Enter(System::Object^ sender, System::EventArgs^ e) {
    textBox2->BackColor = System::Drawing::Color::LightYellow; // Highlight on focus
}

private: System::Void textBox2_Leave(System::Object^ sender, System::EventArgs^ e) {
    textBox2->BackColor = System::Drawing::Color::White; // Reset color when focus is lost
}

};
}