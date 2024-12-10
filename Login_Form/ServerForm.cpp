#include "ServerForm.h"
#include "UserInfo.h"
#include "Client.h"
using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;
using namespace UserInfo;
void recordVideo(int duration = 10)
{
    String^ cmd = "ffmpeg -f dshow -video_size 640x480 -framerate 30";
    cmd += " -rtbufsize 100M";
    cmd += " -i video=\"HD Webcam\""; // Thay bằng tên camera của bạn
    cmd += " -t " + duration.ToString(); // Sử dụng ToString() thay vì std::to_string()
    cmd += " -vcodec h264"; // Đổi codec
    cmd += " -preset ultrafast";
    cmd += " -pix_fmt yuv420p"; // Thêm pixel format
    cmd += " -y output.mp4";

    // Sử dụng Process::Start thay vì system()
    Process::Start("cmd.exe", "/c " + cmd);
}
System::Void LoginForm::ServerForm::switchToClientButton_Click(System::Object^ sender, System::EventArgs^ e) {
    this->Hide();
    String^ userId = UserInfo::User::UserId;
    String^ senderEmail = UserInfo::User::SenderEmail;
    String^ password = UserInfo::User::Password;
    String^ receiverEmail = UserInfo::User::ReceiverEmail;
    String^ applicationKey = UserInfo::User::ApplicationKey;
    Client^ clientForm = gcnew Client(receiverEmail, applicationKey, senderEmail, 0);
    clientForm->Show();
}

namespace LoginForm
{

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
        { // Tạo một đối tượng Bitmap để lưu ảnh chụp màn hình
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
        else if (command->Equals("TAKE_PHOTO", StringComparison::OrdinalIgnoreCase))
        {
            // Use Process to capture photo
            Process^ cameraCaptureProcess = gcnew Process();
            cameraCaptureProcess->StartInfo->FileName = "CommandCam";
            cameraCaptureProcess->StartInfo->Arguments = "/filename captured_photo.jpg";
            cameraCaptureProcess->StartInfo->UseShellExecute = false;
            cameraCaptureProcess->Start();
            cameraCaptureProcess->WaitForExit();

            String^ imagePath = "captured_photo.jpg";
            array<Byte>^ imageData = File::ReadAllBytes(imagePath);

            // Gửi kích thước của mảng byte
            array<Byte>^ sizeBuffer = BitConverter::GetBytes(imageData->Length);
            stream->Write(sizeBuffer, 0, sizeBuffer->Length);
            stream->Flush();

            // Gửi dữ liệu hình ảnh
            stream->Write(imageData, 0, imageData->Length);
            stream->Flush();

            form->UpdateCommunicationLog("Photo sent to client.");
        }
        else if (command->Equals("SEND_VIDEO", StringComparison::OrdinalIgnoreCase))
        {
            // List available devices
            Process::Start("ffmpeg", "-list_devices true -f dshow -i dummy");

            // Record video using Process
            Process^ ffmpegProcess = gcnew Process();
            ffmpegProcess->StartInfo->FileName = "ffmpeg";
            ffmpegProcess->StartInfo->Arguments = "-f dshow -video_size 640x480 -framerate 30 -rtbufsize 100M -i video=\"HD Webcam\" -t 5 -vcodec h264 -preset ultrafast -pix_fmt yuv420p -y output.mp4";
            ffmpegProcess->StartInfo->UseShellExecute = false;
            ffmpegProcess->Start();
            ffmpegProcess->WaitForExit();

            String^ videoPath = "output.mp4";
            array<Byte>^ videoData = File::ReadAllBytes(videoPath);
            array<Byte>^ sizeBuffer = BitConverter::GetBytes(videoData->Length);

            // Gửi kích thước trước
            stream->Write(sizeBuffer, 0, sizeBuffer->Length);
            stream->Flush();

            // Gửi toàn bộ video
            stream->Write(videoData, 0, videoData->Length);
            stream->Flush();

            form->UpdateCommunicationLog("Video sent to client.");
        }
        else if (command->Equals("ENABLE_WIFI", StringComparison::OrdinalIgnoreCase))
        {
            Process::Start("netsh", "interface set interface \"Wi-Fi\" enabled");
            form->UpdateCommunicationLog("Wi-Fi enabled.");
            }
        else if (command->Equals("DISABLE_WIFI", StringComparison::OrdinalIgnoreCase))
        {
            Process::Start("netsh", "interface set interface \"Wi-Fi\" disabled");
            form->UpdateCommunicationLog("Wi-Fi disabled.");
            }
        else if (command->Equals("START_RECORDING", StringComparison::OrdinalIgnoreCase))
        {
            // Đường dẫn tương đối lưu file ghi âm
            String^ relativePath = Application::StartupPath + "\\record.wav";

            Process^ recorder = gcnew Process();
            recorder->StartInfo->FileName = "powershell";
            recorder->StartInfo->Arguments = "-Command \"Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; namespace SoundRecording { public class Audio { [DllImport(\\\"winmm.dll\\\")] public static extern int mciSendString(string command, string buffer, int bufferSize, IntPtr hwndCallback); } }'; [SoundRecording.Audio]::mciSendString('open new Type waveaudio Alias recsound', $null, 0, [IntPtr]::Zero); [SoundRecording.Audio]::mciSendString('record recsound', $null, 0, [IntPtr]::Zero); Start-Sleep -Seconds 60; [SoundRecording.Audio]::mciSendString('save recsound \"" + relativePath + "\"', $null, 0, [IntPtr]::Zero); [SoundRecording.Audio]::mciSendString('close recsound', $null, 0, [IntPtr]::Zero)\"";
            recorder->StartInfo->UseShellExecute = false;
            recorder->StartInfo->RedirectStandardOutput = true;
            recorder->Start();

            form->UpdateCommunicationLog("Recording started for 1 minute and saved as '" + relativePath + "'.");
            }
        else if (action->Equals("START_APP", StringComparison::OrdinalIgnoreCase) && fileName != nullptr)
        {
            Process::Start(fileName);
            form->UpdateCommunicationLog("Started application: " + fileName);
            }


        else if (action->Equals("STOP_APP", StringComparison::OrdinalIgnoreCase) && fileName != nullptr)
        {
            Process^ killProcess = gcnew Process();
            killProcess->StartInfo->FileName = "taskkill";
            killProcess->StartInfo->Arguments = "/IM " + fileName + " /F";
            killProcess->StartInfo->UseShellExecute = false;
            killProcess->Start();
            form->UpdateCommunicationLog("Stopped application: " + fileName);
            }
        else if (command->Equals("GET_CPU_INFO", StringComparison::OrdinalIgnoreCase))
        {
            // Lấy thông tin CPU
            Process^ cpuInfoProcess = gcnew Process();
            cpuInfoProcess->StartInfo->FileName = "wmic";
            cpuInfoProcess->StartInfo->Arguments = "cpu get name";
            cpuInfoProcess->StartInfo->UseShellExecute = false;
            cpuInfoProcess->StartInfo->RedirectStandardOutput = true;
            cpuInfoProcess->Start();

            output = cpuInfoProcess->StandardOutput->ReadToEnd();
            cpuInfoProcess->WaitForExit();

            form->UpdateCommunicationLog("CPU Information:\n" + output);
            }


        else if (action->Equals("SET_VOLUME", StringComparison::OrdinalIgnoreCase) && commandParts->Length > 1)
        {
            String^ volume = commandParts[1];
            Process::Start("nircmd.exe", "setsysvolume " + volume);
            form->UpdateCommunicationLog("Volume set to: " + volume + "%");
            }


        else if (action->Equals("GET_FILE", StringComparison::OrdinalIgnoreCase) && fileName != nullptr)
        {
            if (System::IO::File::Exists(fileName))
            {
                try
                {
                    // Đọc dữ liệu từ file
                    array<Byte>^ fileData = System::IO::File::ReadAllBytes(fileName);

                    // Chuyển đổi tên file sang UTF-8 để truyền đúng ký tự đặc biệt và tiếng Việt
                    array<Byte>^ fileNameBytes = Encoding::UTF8->GetBytes(fileName);
                    int fileNameLength = fileNameBytes->Length;

                    // Gửi độ dài của tên file (4 byte)
                    array<Byte>^ nameLengthBuffer = BitConverter::GetBytes(fileNameLength);
                    stream->Write(nameLengthBuffer, 0, nameLengthBuffer->Length);
                    stream->Flush(); // Đảm bảo dữ liệu đã được gửi

                    // Gửi tên file
                    stream->Write(fileNameBytes, 0, fileNameBytes->Length);
                    stream->Flush(); // Đảm bảo tên file đã được gửi

                    // Gửi kích thước của file (4 byte)
                    array<Byte>^ sizeBuffer = BitConverter::GetBytes(fileData->Length);
                    stream->Write(sizeBuffer, 0, sizeBuffer->Length);
                    stream->Flush(); // Đảm bảo kích thước đã được gửi

                    // Gửi nội dung file
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
        //else if (action->Equals("SEND_EMAIL", StringComparison::OrdinalIgnoreCase) && commandParts->Length >= 4)
        //{
        //    String^ recipient = commandParts[1]; // Địa chỉ email người nhận
        //    String^ subject = commandParts[2];   // Chủ đề email
        //    String^ body = String::Join(" ", commandParts, 3, commandParts->Length - 3); // Nội dung email

        //    try
        //    {
        //        // Cấu hình đối tượng SmtpClient
        //        SmtpClient^ smtpClient = gcnew SmtpClient("smtp.gmail.com", 587);
        //        smtpClient->Credentials = gcnew NetworkCredential("duyminh121220051@gmail.com", "ifvpqrjuoibjwafn"); // Sử dụng thông tin đăng nhập của bạn
        //        smtpClient->EnableSsl = true; // Kích hoạt SSL

        //        // Tạo đối tượng MailMessage
        //        MailMessage^ mailMessage = gcnew MailMessage();
        //        mailMessage->From = gcnew MailAddress("duyminh121220051@gmail.com"); // Địa chỉ email gửi
        //        mailMessage->To->Add(recipient); // Địa chỉ email người nhận
        //        mailMessage->Subject = subject; // Chủ đề email
        //        mailMessage->Body = body; // Nội dung email

        //        // Gửi email
        //        smtpClient->Send(mailMessage);
        //        form->UpdateCommunicationLog("Email sent to: " + recipient);
        //    }
        //    catch (Exception^ ex)
        //    {
        //        form->UpdateCommunicationLog("Failed to send email: " + ex->Message);
        //    }
        //    }
        // else
        //{
        //     form->UpdateCommunicationLog("Unknown command: " + command);
        // }
    }

    void ServerForm::checkBoxDarkMode_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
    {
        if (this->checkBoxDarkMode->Checked) // Chế độ tối
        {
            this->BackColor = System::Drawing::Color::FromArgb(45, 45, 48);                      // Màu nền tối
            this->textBoxServerOutput->BackColor = System::Drawing::Color::FromArgb(30, 30, 30); // Màu nền hộp text
            this->textBoxServerOutput->ForeColor = System::Drawing::Color::White;                // Màu chữ
            this->buttonStartServer->BackColor = System::Drawing::Color::DarkOliveGreen;         // Màu nút
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
        // progressBarServerStatus->Value = 50; // cập nhật giá trị khi server đang khởi động
        pictureBoxStatus->BackColor = System::Drawing::Color::LightGreen;

        Thread^ serverThread = gcnew Thread(gcnew ThreadStart(this, &ServerForm::StartServer));
        serverThread->IsBackground = true; // Đảm bảo luồng này kết thúc khi ứng dụng chính đóng
        serverThread->Start();
    }
    void ServerForm::StartServer()
    {

        TcpListener^ listener = gcnew TcpListener(IPAddress::Any, 12345); // Lắng nghe trên tất cả các địa chỉ IP

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
            array<Byte>^ passwordBuffer = gcnew array<Byte>(256);
            int passwordBytes = stream->Read(passwordBuffer, 0, passwordBuffer->Length);
            String^ receivedPassword = Encoding::UTF8->GetString(passwordBuffer, 0, passwordBytes)->Trim();
            this->UpdateCommunicationLog("Expected password: " + User::Password);


            if (receivedPassword != User::Password)
            {
                this->UpdateCommunicationLog("Client sent incorrect password. Connection rejected.");

                // Gửi phản hồi số 0 (mật khẩu sai)
                array<Byte>^ response = gcnew array<Byte>(1);
                response[0] = false; // 0 cho mật khẩu sai
                stream->Write(response, 0, response->Length);

                // Đóng kết nối
                stream->Close();
                client->Close();
                continue; // Quay lại chấp nhận kết nối mới
            }
            array<Byte>^ successResponse = gcnew array<Byte>(1);
            successResponse[0] = true; // 1 cho mật khẩu đúng
            stream->Write(successResponse, 0, successResponse->Length);

            this->UpdateCommunicationLog("Password verified. Connection accepted.");


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