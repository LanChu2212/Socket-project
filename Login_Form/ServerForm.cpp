#include "ServerForm.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;
namespace LoginForm {

    void ServerForm::UpdateCommunicationLog(String^ message)
    {
        if (this->textBoxServerOutput->InvokeRequired) // Kiểm tra xem có cần Invoke không
        {
            this->textBoxServerOutput->Invoke(gcnew Action<String^>(this, &ServerForm::UpdateCommunicationLog), message);
        }
        else
        {
            this->textBoxServerOutput->AppendText(message + "\r\n");
        }
    }

    // Hàm thực hiện các lệnh nhận được
    void ExecuteCommand(String^ command, ServerForm^ form, NetworkStream^ stream)
    {
        command = command->Trim();
        String^ output;
        array<String^>^ buffer;
        // Phân tích cú pháp lệnh
        array<String^>^ commandParts = command->Split(' ');
        String^ action = commandParts[0];
        String^ fileName = commandParts->Length > 1 ? commandParts[1] : nullptr;

        if (command->Equals("LIST_APPS", StringComparison::OrdinalIgnoreCase))
        {
            Process^ processList = gcnew Process();
            processList->StartInfo->FileName = "tasklist";
            processList->StartInfo->UseShellExecute = false;
            processList->StartInfo->RedirectStandardOutput = true;
            processList->Start();

            output = processList->StandardOutput->ReadToEnd();
            processList->WaitForExit();

            form->UpdateCommunicationLog("Running apps:\n" + output);
            array<Byte>^ buffer = Encoding::UTF8->GetBytes(output);
            stream->Write(buffer, 0, buffer->Length);
        }
        else if (command->Equals("LIST_SERVICES", StringComparison::OrdinalIgnoreCase))
        {   
           
            Process^ serviceList = gcnew Process();
            serviceList->StartInfo->FileName = "sc";
            serviceList->StartInfo->Arguments = "query";
            serviceList->StartInfo->UseShellExecute = false;
            serviceList->StartInfo->RedirectStandardOutput = true;
            serviceList->Start();

            output = serviceList->StandardOutput->ReadToEnd();
            serviceList->WaitForExit();

            form->UpdateCommunicationLog("Services:\n" + output);
            array<Byte>^ buffer = Encoding::UTF8->GetBytes(output);
            stream->Write(buffer, 0, buffer->Length);
        }
        else if (command->Equals("SHUTDOWN", StringComparison::OrdinalIgnoreCase))
        {
            String^ response = "Server is shutting down...";
            array<Byte>^ responseData = Encoding::UTF8->GetBytes(response);
            stream->Write(responseData, 0, responseData->Length);

            // Cập nhật log hoặc thực hiện hành động khác nếu cần
            form->UpdateCommunicationLog(response);

            Process::Start("shutdown", "/s /t 0");
           
        }
        else if (command->Equals("GET_DATE", StringComparison::OrdinalIgnoreCase))
        {
            output = DateTime::Now.ToString("yyyy-MM-dd");
            form->UpdateCommunicationLog("Current Date: " + output);
        }
        else if (command->Equals("GET_TIME", StringComparison::OrdinalIgnoreCase))
        {
            output = DateTime::Now.ToString("HH:mm:ss");
            form->UpdateCommunicationLog("Current Time: " + output);
        }
        else if (command->Equals("GET_IP", StringComparison::OrdinalIgnoreCase))
        {
            String^ ipAddress = Dns::GetHostEntry(Dns::GetHostName())->AddressList[0]->ToString();
            form->UpdateCommunicationLog("Current IP Address: " + ipAddress);
        }
        else if (command->Equals("GET_OS", StringComparison::OrdinalIgnoreCase))
        {
            output = Environment::OSVersion->ToString();
            form->UpdateCommunicationLog("Operating System: " + output);
        }
        else if (command->Equals("OPEN_GOOGLE", StringComparison::OrdinalIgnoreCase))
        {
            Process::Start("https://www.google.com");
            form->UpdateCommunicationLog("Opened Google in default browser.");
        }
        else if (command->Equals("TAKE_SCREENSHOT", StringComparison::OrdinalIgnoreCase))
        {// Tạo một đối tượng Bitmap để lưu ảnh chụp màn hình
            Bitmap^ screenshot = gcnew Bitmap(Screen::PrimaryScreen->Bounds.Width, Screen::PrimaryScreen->Bounds.Height);
            Graphics^ g = Graphics::FromImage(screenshot);

            // Chụp màn hình
            g->CopyFromScreen(0, 0, 0, 0, screenshot->Size);

            // Lưu ảnh vào một MemoryStream dưới định dạng PNG
            MemoryStream^ ms = gcnew MemoryStream();
            screenshot->Save(ms, Imaging::ImageFormat::Png);
            array<Byte>^ imageData = ms->ToArray();

            // Gửi kích thước của mảng byte
            array<Byte>^ sizeBuffer = BitConverter::GetBytes(imageData->Length);
            stream->Write(sizeBuffer, 0, sizeBuffer->Length);
            stream->Flush(); // Đảm bảo kích thước đã được gửi

            // Gửi dữ liệu hình ảnh
            stream->Write(imageData, 0, imageData->Length);
            stream->Flush(); // Đảm bảo dữ liệu hình ảnh đã được gửi

            // Giải phóng tài nguyên
            delete g;
            delete screenshot;
            ms->Close();

            form->UpdateCommunicationLog("Screenshot sent to client.");
        }
        else if (action->Equals("GET_FILE", StringComparison::OrdinalIgnoreCase) && fileName != nullptr)
        {
            if (System::IO::File::Exists(fileName))
            {
                try
                {
                    // Đọc dữ liệu từ file
                    array<Byte>^ fileData = System::IO::File::ReadAllBytes(fileName);

                    // Gửi kích thước của file cho client trước
                    array<Byte>^ sizeBuffer = BitConverter::GetBytes(fileData->Length);
                    stream->Write(sizeBuffer, 0, sizeBuffer->Length);
                    stream->Flush(); // Đảm bảo kích thước đã được gửi

                    // Gửi nội dung của file
                    stream->Write(fileData, 0, fileData->Length);
                    stream->Flush(); // Đảm bảo dữ liệu file đã được gửi

                    form->UpdateCommunicationLog("File " + fileName + " sent to client.");
                }
                catch (Exception^ ex)
                {
                    form->UpdateCommunicationLog("Error sending file: " + ex->Message);
                }
            }
            else
            {
                // Nếu không tìm thấy file
                form->UpdateCommunicationLog("File not found: " + fileName);
                String^ errorMessage = "ERROR: File not found";
                array<Byte>^ errorData = Encoding::UTF8->GetBytes(errorMessage);
                stream->Write(errorData, 0, errorData->Length);
                stream->Flush(); // Đảm bảo thông báo lỗi đã được gửi
            }
        }
        else if (action->Equals("DELETE_FILE", StringComparison::OrdinalIgnoreCase) && fileName != nullptr)
        {
            // Xóa file
            if (System::IO::File::Exists(fileName))
            {
                System::IO::File::Delete(fileName);
                form->UpdateCommunicationLog("File deleted: " + fileName);
            }
            else
            {
                form->UpdateCommunicationLog("File not found: " + fileName);
            }
        }
        //else
        //{
        //    form->UpdateCommunicationLog("Unknown command: " + command);
        //}
    }
    
    
    
  

    void ServerForm::checkBoxDarkMode_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
    {
        if (this->checkBoxDarkMode->Checked) // Chế độ tối
        {
            this->BackColor = System::Drawing::Color::FromArgb(45, 45, 48); // Màu nền tối
            this->textBoxServerOutput->BackColor = System::Drawing::Color::FromArgb(30, 30, 30); // Màu nền hộp text
            this->textBoxServerOutput->ForeColor = System::Drawing::Color::White; // Màu chữ
            this->buttonStartServer->BackColor = System::Drawing::Color::DarkOliveGreen; // Màu nút
            this->buttonStopServer->BackColor = System::Drawing::Color::Firebrick;
            this->labelStatus->ForeColor = System::Drawing::Color::White; // Màu chữ trạng thái
        }
        else // Chế độ sáng
        {
            this->BackColor = System::Drawing::Color::White; // Màu nền sáng
            this->textBoxServerOutput->BackColor = System::Drawing::Color::AliceBlue;
            this->textBoxServerOutput->ForeColor = System::Drawing::Color::Black;
            this->buttonStartServer->BackColor = System::Drawing::Color::LightGreen;
            this->buttonStopServer->BackColor = System::Drawing::Color::Tomato;
            this->labelStatus->ForeColor = System::Drawing::Color::Gray; // Màu chữ mặc định
        }
    }


    // Hàm khởi động server
    System::Void ServerForm::buttonStartServer_Click(System::Object^ sender, System::EventArgs^ e)
    {

        this->labelStatus->Text = "Status: Server is running";
        this->UpdateCommunicationLog("Server started...");
        //progressBarServerStatus->Value = 50; // cập nhật giá trị khi server đang khởi động
        pictureBoxStatus->BackColor = System::Drawing::Color::LightGreen;

        Thread^ serverThread = gcnew Thread(gcnew ThreadStart(this, &ServerForm::StartServer));
        serverThread->IsBackground = true; // Đảm bảo luồng này kết thúc khi ứng dụng chính đóng
        serverThread->Start();
    }
    void ServerForm::StartServer()
    {

        TcpListener^ listener = gcnew TcpListener(IPAddress::Parse("127.0.0.1"), 12345);
        listener->Start();

        this->UpdateCommunicationLog("Server is waiting for commands...");

        while (true)
        {
            TcpClient^ client = listener->AcceptTcpClient();
            IPEndPoint^ clientEndPoint = dynamic_cast<IPEndPoint^>(client->Client->RemoteEndPoint);

            // Display the client's IP address and port
            String^ clientInfo = "Client connected from: " + clientEndPoint->Address->ToString() +
                ":" + clientEndPoint->Port.ToString();
            this->UpdateCommunicationLog(clientInfo);

            NetworkStream^ stream = client->GetStream();

            array<Byte>^ data = gcnew array<Byte>(256);
            int bytes = stream->Read(data, 0, data->Length);
            String^ command = Encoding::UTF8->GetString(data, 0, bytes);
            command = command->Trim();

            this->UpdateCommunicationLog("Received command: " + command);
            ExecuteCommand(command, this, stream); // Gọi hàm ExecuteCommand

            stream->Close();
            client->Close();
        }

        listener->Stop();
    }


    // Hàm dừng server
    System::Void ServerForm::buttonStopServer_Click(System::Object^ sender, System::EventArgs^ e)
    {
        this->labelStatus->Text = "Status: Server is stopped";
        pictureBoxStatus->BackColor = System::Drawing::Color::Tomato;
        this->UpdateCommunicationLog("Server stopped...");
    }
}