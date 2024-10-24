
using namespace System;
using namespace System::Globalization;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Net;
using namespace System::Net::Mail;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace EAGetMail; // Thư viện EAGetMail cho email

// Hàm tạo tên file tạm thời (có thể không cần trong trường hợp này nếu bạn không lưu file)
static String^ GenerateFileName(int sequence)
{
    DateTime currentDateTime = DateTime::Now;
    return String::Format("{0}-{1:000}-{2:000}.txt",
        currentDateTime.ToString("yyyyMMddHHmmss", gcnew CultureInfo("en-US")),
        currentDateTime.Millisecond,
        sequence);
}

// Hàm tìm thư mục Inbox
static Imap4Folder^ FindInboxFolder(array<Imap4Folder^>^ folders)
{
    for each (Imap4Folder ^ folder in folders)
    {
        if (folder->Name->Equals("INBOX", StringComparison::OrdinalIgnoreCase))
        {
            return folder;
        }
    }
    return nullptr;
}

// Hàm kiểm tra xem email đã được xử lý chưa
static bool IsEmailProcessed(String^ messageId, String^ processedEmailsFile)
{
    if (!File::Exists(processedEmailsFile))
    {
        return false;  // Nếu file chưa tồn tại, email chưa được xử lý
    }

    array<String^>^ processedEmails = File::ReadAllLines(processedEmailsFile);
    for each (String ^ id in processedEmails)
    {
        if (id->Equals(messageId))
        {
            return true;  // Email đã được xử lý
        }
    }
    return false;
}

// Hàm lưu lại email đã xử lý
static void SaveProcessedEmail(String^ messageId, String^ processedEmailsFile)
{
    File::AppendAllText(processedEmailsFile, messageId + Environment::NewLine);
}

static bool IsEmailFromSpecificSender(EAGetMail::Mail^ oMail, String^ specificSender)
{
    // So sánh địa chỉ email của người gửi với địa chỉ cụ thể
    return oMail->From->Address->Equals(specificSender, StringComparison::OrdinalIgnoreCase);
}

static void ReceiveImage(NetworkStream^ stream)
{
    // Nhận kích thước của dữ liệu hình ảnh
    array<Byte>^ sizeBuffer = gcnew array<Byte>(4);
    stream->Read(sizeBuffer, 0, sizeBuffer->Length);

    // Chuyển đổi kích thước từ byte sang int
    int imageSize = BitConverter::ToInt32(sizeBuffer, 0);

    // Nhận dữ liệu hình ảnh
    array<Byte>^ imageData = gcnew array<Byte>(imageSize);
    stream->Read(imageData, 0, imageData->Length);

    // Lưu dữ liệu hình ảnh vào file
    String^ imagePath = "screenshot_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".png"; // Đặt tên cho file hình ảnh
    File::WriteAllBytes(imagePath, imageData);

    Console::WriteLine("Image received and saved as: " + imagePath);
}
void SendEmail(String^ recipient, String^ subject, String^ body, String^ attachmentPath)
{
    try
    {
        // Cấu hình thông tin email
        SmtpClient^ smtpClient = gcnew SmtpClient("smtp.gmail.com", 587);
        smtpClient->Credentials = gcnew NetworkCredential("nguyenlamphuquykh@gmail.com", "benmaankriqyrnfy");
        smtpClient->EnableSsl = true;

        // Tạo một đối tượng MailMessage
        MailMessage^ mailMessage = gcnew MailMessage();
        System::Net::Mail::MailAddress^ mailAddress = gcnew System::Net::Mail::MailAddress("nguyenlamphuquykh@gmail.com");
        mailMessage->From = mailAddress;
        mailMessage->To->Add(recipient);
        mailMessage->Subject = subject;
        mailMessage->Body = body;

        // Nếu có tệp đính kèm
        if (!String::IsNullOrEmpty(attachmentPath))
        {
            mailMessage->Attachments->Add(gcnew System::Net::Mail::Attachment(attachmentPath));
        }

        // Gửi email
        smtpClient->Send(mailMessage);
        Console::WriteLine("Email sent successfully to " + recipient);
    }
    catch (Exception^ ex)
    {
        Console::WriteLine("Error sending email: " + ex->Message);
    }
}


static void handleResponse(String^ command, NetworkStream^ stream, String^ recipientEmail) {
    if (command->Equals("TAKE_SCREENSHOT", StringComparison::OrdinalIgnoreCase))
    {   
        Console::WriteLine(command);
        array<Byte>^ sizeBuffer = gcnew array<Byte>(4);
        int bytesRead = stream->Read(sizeBuffer, 0, sizeBuffer->Length);
        if (bytesRead != sizeBuffer->Length)
        {
            Console::WriteLine("Failed to receive the image size.");
            return;
        }

        // Chuyển đổi kích thước từ byte sang int
        int imageSize = BitConverter::ToInt32(sizeBuffer, 0);
        if (imageSize <= 0)
        {
            Console::WriteLine("Invalid image size received.");
            return;
        }
        else{
            Console::WriteLine(imageSize);
        }

        // Nhận dữ liệu hình ảnh
        array<Byte>^ imageData = gcnew array<Byte>(imageSize);
        int totalBytesRead = 0;
        while (totalBytesRead < imageSize)
        {
            int read = stream->Read(imageData, totalBytesRead, imageSize - totalBytesRead);
            if (read <= 0)
            {
                Console::WriteLine("Failed to receive the complete image data.");
                return;
            }
            totalBytesRead += read;
        }

        // Lưu dữ liệu hình ảnh vào file
        String^ imagePath = "screenshot_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".png";
        File::WriteAllBytes(imagePath, imageData);
        Console::WriteLine("Image size received: " + imageSize);
        Console::WriteLine("Image received and saved as: " + imagePath);

        String^ subject = "Screenshot from Server";
        String^ body = "Here is the screenshot you requested.";
        SendEmail(recipientEmail, subject, body, imagePath);
    }
    else if (command->Contains("GET_FILE")) {
        // Bước 1: Gửi yêu cầu nhận file đến server
        Console::WriteLine("Requesting file from server...");

        // Bước 2: Nhận kích thước của file từ server
        array<Byte>^ sizeBuffer = gcnew array<Byte>(4);
        int bytesRead = stream->Read(sizeBuffer, 0, sizeBuffer->Length);
        if (bytesRead != sizeBuffer->Length)
        {
            Console::WriteLine("Failed to receive the file size.");
            return;
        }

        // Chuyển đổi kích thước từ byte sang int
        int fileSize = BitConverter::ToInt32(sizeBuffer, 0);
        if (fileSize <= 0)
        {
            Console::WriteLine("Invalid file size received.");
            return;
        }
        else
        {
            Console::WriteLine("File size: " + fileSize);
        }

        // Bước 3: Nhận dữ liệu file từ server
        array<Byte>^ fileData = gcnew array<Byte>(fileSize);
        int totalBytesRead = 0;
        while (totalBytesRead < fileSize)
        {
            int read = stream->Read(fileData, totalBytesRead, fileSize - totalBytesRead);
            if (read <= 0)
            {
                Console::WriteLine("Failed to receive the complete file data.");
                return;
            }
            totalBytesRead += read;
        }

        // Bước 4: Lưu dữ liệu file vào file
        String^ filePath = "received_file_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".dat"; // Đặt tên cho file
        File::WriteAllBytes(filePath, fileData);
        Console::WriteLine("File received and saved as: " + filePath);

        // Gửi email thông báo cho người dùng với file đã nhận làm tệp đính kèm
        String^ subject = "File Received from Server";
        String^ body = "Here is the file you requested.";
        SendEmail(recipientEmail, subject, body, filePath);
    }
    else {
        array<Byte>^ buffer = gcnew array<Byte>(4096); // Tạo bộ đệm để lưu dữ liệu nhận về
        int bytesRead = stream->Read(buffer, 0, buffer->Length); // Đọc dữ liệu từ server

        if (bytesRead > 0)
        {
            String^ response = Encoding::UTF8->GetString(buffer, 0, bytesRead);
            Console::WriteLine("Response from server:\r\n{0}", response);
            String^ subject = "Response from Server";
            String^ body = "Here is the list of running apps/services";
            SendEmail(recipientEmail, subject, body, response);
        }
        else
        {
            Console::WriteLine("No response from server.");
        }
    }
}

int main(array<System::String^>^ args)
{
    try
    {
        // Lưu lại thời gian khi ứng dụng khởi động
        DateTime startTime = DateTime::Now;

        // Địa chỉ email của người gửi cụ thể cần lọc (có thể để trống nếu không cần lọc theo người gửi)
        String^ specificSender = "nguyenlamphuquynt@gmail.com";

        // Thư mục tạm cho email đã xử lý
        String^ processedEmailsFile = Path::Combine(Directory::GetCurrentDirectory(), "processed_emails.txt");

        // Cấu hình máy chủ Gmail IMAP
        MailServer^ oServer = gcnew MailServer("imap.gmail.com",
            "nguyenlamphuquykh@gmail.com",  // Địa chỉ email của bạn
            "rnsbuhslquffkvmo",     // Mật khẩu ứng dụng
            ServerProtocol::Imap4);  // Giao thức IMAP

        oServer->SSLConnection = true;  // Sử dụng kết nối SSL/TLS
        oServer->Port = 993;            // Cổng SSL/TLS IMAP

        Console::WriteLine("Connecting to IMAP server...");

        // Tạo đối tượng MailClient
        MailClient^ oClient = gcnew MailClient("TryIt");

        while (true)  // Liên tục kiểm tra email mới
        {
            try
            {
                oClient->Connect(oServer);
                array<Imap4Folder^>^ folders = oClient->GetFolders();
                Imap4Folder^ inboxFolder = FindInboxFolder(folders);

                if (inboxFolder != nullptr)
                {
                    oClient->SelectFolder(inboxFolder);
                    array<MailInfo^>^ infos = oClient->GetMailInfos();
                    Console::WriteLine("Total {0} email(s)\r\n", infos->Length);

                    if (infos->Length > 0)
                    {
                        for (int i = infos->Length - 1; i >= 0; i--)
                        {
                            MailInfo^ mailInfo = infos[i];
                            String^ messageId = mailInfo->UIDL;  // Sử dụng UIDL để phân biệt email

                            // Kiểm tra xem email đã được xử lý chưa
                            if (!IsEmailProcessed(messageId, processedEmailsFile))
                            {
                                try
                                {
                                    // Lấy nội dung email
                                    EAGetMail::Mail^ oMail = oClient->GetMail(mailInfo);

                                    // Kiểm tra thời gian gửi email
                                   

                                    // Kiểm tra người gửi có khớp với địa chỉ cụ thể không (nếu cần thiết)
                                    if (specificSender->Length == 0 || IsEmailFromSpecificSender(oMail, specificSender))
                                    {
                                        Console::WriteLine("From: {0}", oMail->From->ToString());
                                        Console::WriteLine("Subject: {0}\r\n", oMail->Subject);
                                        Console::WriteLine("Body: {0}\r\n", oMail->TextBody); // In ra nội dung email

                                        // Gửi lệnh từ nội dung email đến server điều khiển (giả sử qua socket)
                                        // Gửi lệnh từ nội dung email đến server điều khiển (giả sử qua socket)
                                        String^ command = oMail->TextBody->Trim();

                                        // Thực thi lệnh qua socket (giả sử đang kết nối với server qua socket)
                                        TcpClient^ client = gcnew TcpClient("127.0.0.1", 12345); // Địa chỉ IP server
                                        NetworkStream^ stream = client->GetStream();

                                        // Gửi lệnh đến server
                                        array<Byte>^ data = Encoding::UTF8->GetBytes(command);
                                        stream->Write(data, 0, data->Length);
                                        stream->Flush(); // Đảm bảo dữ liệu đã được gửi đi
                                        handleResponse(command, stream, specificSender);
                                        

                                        // Đóng kết nối
                                        stream->Close();
                                        client->Close();

                                        // Đánh dấu email đã đọc nếu cần thiết
                                        oClient->MarkAsRead(mailInfo, true);

                                        // Lưu lại email đã xử lý
                                        SaveProcessedEmail(messageId, processedEmailsFile);
                                    }
                                    else
                                    {
                                        Console::WriteLine("Email is not from the specified sender: {0}\r\n", oMail->From->ToString());
                                    }
                                }
                                catch (Exception^ e)
                                {
                                    Console::WriteLine("Failed to process email {0}: {1}", mailInfo->Index, e->Message);
                                }
                            }
                            else
                            {
                                Console::WriteLine("Email already processed: {0}\r\n", messageId);
                            }
                        }
                    }
                    else
                    {
                        Console::WriteLine("No emails found!");
                    }
                }
                else
                {
                    Console::WriteLine("Inbox folder not found!");
                }

                oClient->Quit();
                Console::WriteLine("Waiting for the next check...");
            }
            catch (Exception^ e)
            {
                Console::WriteLine("Error: {0}", e->Message);
            }

            // Sleep for 60 seconds before checking again
            // Thread::Sleep(60000);  // 60 seconds
        }
    }
    catch (Exception^ ep)
    {
        Console::WriteLine("Error: {0}", ep->Message);
    }

    return 0;
}

