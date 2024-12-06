using namespace System;
using namespace System::Globalization;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Net;
using namespace System::Net::Mail;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace EAGetMail; // Thư viện EAGetMail cho email
#include <iostream>
// Hàm tạo tên file tạm thời (có thể không cần trong trường hợp này nếu bạn không lưu file)
static String ^ GenerateFileName(int sequence) {
    DateTime currentDateTime = DateTime::Now;
    return String::Format("{0}-{1:000}-{2:000}.txt",
                          currentDateTime.ToString("yyyyMMddHHmmss", gcnew CultureInfo("en-US")),
                          currentDateTime.Millisecond,
                          sequence);
}

    // Hàm tìm thư mục Inbox
    static Imap4Folder
    ^ FindInboxFolder(array<Imap4Folder ^> ^ folders) {
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
    static bool IsEmailProcessed(String ^ messageId, String ^ processedEmailsFile)
{
    if (!File::Exists(processedEmailsFile))
    {
        return false; // Nếu file chưa tồn tại, email chưa được xử lý
    }

    array<String ^> ^ processedEmails = File::ReadAllLines(processedEmailsFile);
    for each (String ^ id in processedEmails)
    {
        if (id->Equals(messageId))
        {
            return true; // Email đã được xử lý
        }
    }
    return false;
}

// Hàm lưu lại email đã xử lý
static void SaveProcessedEmail(String ^ messageId, String ^ processedEmailsFile)
{
    File::AppendAllText(processedEmailsFile, messageId + Environment::NewLine);
}

static bool IsEmailFromSpecificSender(EAGetMail::Mail ^ oMail, String ^ specificSender)
{
    // So sánh địa chỉ email của người gửi với địa chỉ cụ thể
    return oMail->From->Address->Equals(specificSender, StringComparison::OrdinalIgnoreCase);
}

static void ReceiveImage(NetworkStream ^ stream)
{
    // Nhận kích thước của dữ liệu hình ảnh
    array<Byte> ^ sizeBuffer = gcnew array<Byte>(4);
    stream->Read(sizeBuffer, 0, sizeBuffer->Length);

    // Chuyển đổi kích thước từ byte sang int
    int imageSize = BitConverter::ToInt32(sizeBuffer, 0);

    // Nhận dữ liệu hình ảnh
    array<Byte> ^ imageData = gcnew array<Byte>(imageSize);
    stream->Read(imageData, 0, imageData->Length);

    // Lưu dữ liệu hình ảnh vào file
    String ^ imagePath = "screenshot_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".png"; // Đặt tên cho file hình ảnh
    File::WriteAllBytes(imagePath, imageData);

    Console::WriteLine("Image received and saved as: " + imagePath);
}
void SendEmail(String ^ recipient, String ^ subject, String ^ body, String ^ attachmentPath)
{
    try
    {
        // Kiểm tra thông tin người nhận
        if (String::IsNullOrEmpty(recipient) || !System::Text::RegularExpressions::Regex::IsMatch(recipient, "^\\S+@\\S+\\.\\S+$"))
        {
            throw gcnew ArgumentException("Địa chỉ email không hợp lệ: " + recipient);
        }

        // Kiểm tra nội dung email
        if (String::IsNullOrEmpty(subject))
        {
            subject = "No Subject";
        }
        if (String::IsNullOrEmpty(body))
        {
            body = "No Content";
        }

        // Cấu hình thông tin email
        SmtpClient ^ smtpClient = gcnew SmtpClient("smtp.gmail.com", 587);
        smtpClient->Credentials = gcnew NetworkCredential("huynhtrungkiet09032005@gmail.com", "ifztigzwrspjwhch");
        smtpClient->EnableSsl = true;

        // Tạo một đối tượng MailMessage
        MailMessage ^ mailMessage = gcnew MailMessage();
        System::Net::Mail::MailAddress ^ mailAddress = gcnew System::Net::Mail::MailAddress("huynhtrungkiet09032005@gmail.com");
        mailMessage->From = mailAddress;
        mailMessage->To->Add(recipient);
        mailMessage->Subject = subject;
        mailMessage->Body = body;

        // Kiểm tra và thêm tệp đính kèm
        if (!String::IsNullOrEmpty(attachmentPath))
        {
            if (System::IO::File::Exists(attachmentPath))
            {
                mailMessage->Attachments->Add(gcnew System::Net::Mail::Attachment(attachmentPath));
            }
            else
            {
                String ^ tempFilePath = "response.txt";

                // Tạo đối tượng StreamWriter để ghi vào tệp
                StreamWriter ^ writer = gcnew StreamWriter(tempFilePath);
                writer->Write(attachmentPath); // Ghi chuỗi vào tệp
                writer->Close();               // Đóng tệp sau khi ghi

                mailMessage->Attachments->Add(gcnew System::Net::Mail::Attachment(tempFilePath));

                // mailMessage->Body += attachmentPath;
            }
        }

        // Gửi email
        // Console::WriteLine("Email content is " + attachmentPath);
        smtpClient->Send(mailMessage);
        Console::WriteLine("Email sent successfully to " + recipient);
    }
    catch (Exception ^ ex)
    {
        Console::WriteLine("Error sending email: " + ex->Message);
        Console::WriteLine("Stack Trace: " + ex->StackTrace);
    }
}
static String ^ ExtractIpFromEmailBody(String ^ emailBody) {
    // Giả sử địa chỉ IP nằm ở đầu nội dung email, sau chuỗi "IP: "
    int ipStartIndex = emailBody->IndexOf("IP: ");
    if (ipStartIndex == -1)
    {
        return nullptr; // Không tìm thấy địa chỉ IP
    }

    int ipEndIndex = emailBody->IndexOf(Environment::NewLine, ipStartIndex);
    if (ipEndIndex == -1)
    {
        ipEndIndex = emailBody->Length; // Địa chỉ IP kéo dài đến hết nội dung
    }

    return emailBody->Substring(ipStartIndex + 4, ipEndIndex - ipStartIndex - 4); // Trả về địa chỉ IP
}

    static String
    ^ ExtractCommandFromEmailBody(String ^ emailBody) {
          // Kiểm tra xem emailBody có hợp lệ không
          if (String::IsNullOrEmpty(emailBody))
          {
              return nullptr; // Nếu emailBody rỗng hoặc null, không có lệnh
          }

          // Tìm vị trí của ký tự xuống dòng đầu tiên
          int firstNewLineIndex = emailBody->IndexOf(Environment::NewLine);
          if (firstNewLineIndex == -1)
          {
              return nullptr; // Nếu không tìm thấy dấu xuống dòng, không có lệnh
          }

          // Bỏ qua dòng đầu tiên (dòng chứa địa chỉ IP) và lấy phần còn lại
          String ^ remainingBody = emailBody->Substring(firstNewLineIndex + Environment::NewLine->Length)->Trim();

          return remainingBody;
      } static void handleResponse(String ^ command, NetworkStream ^ stream, String ^ recipientEmail)
{
    if (command->Equals("TAKE_SCREENSHOT", StringComparison::OrdinalIgnoreCase))
    {
        Console::WriteLine(command);
        array<Byte> ^ sizeBuffer = gcnew array<Byte>(4);
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
        else
        {
            Console::WriteLine(imageSize);
        }

        // Nhận dữ liệu hình ảnh
        array<Byte> ^ imageData = gcnew array<Byte>(imageSize);
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
        String ^ imagePath = "screenshot_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".png";
        File::WriteAllBytes(imagePath, imageData);
        Console::WriteLine("Image size received: " + imageSize);
        Console::WriteLine("Image received and saved as: " + imagePath);

        String ^ subject = "Screenshot from Server";
        String ^ body = "Here is the screenshot you requested.";
        SendEmail(recipientEmail, subject, body, imagePath);
    }
    else if (command->Equals("TAKE_PHOTO", StringComparison::OrdinalIgnoreCase))
    {
        Console::WriteLine(command);
        array<Byte> ^ sizeBuffer = gcnew array<Byte>(4);
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
        else
        {
            Console::WriteLine(imageSize);
        }

        // Nhận dữ liệu hình ảnh
        array<Byte> ^ imageData = gcnew array<Byte>(imageSize);
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
        String ^ imagePath = "Photo_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".png";
        File::WriteAllBytes(imagePath, imageData);
        Console::WriteLine("Image size received: " + imageSize);
        Console::WriteLine("Image received and saved as: " + imagePath);

        String ^ subject = "Photo from Server";
        String ^ body = "Here is the Photo you requested.";
        SendEmail(recipientEmail, subject, body, imagePath);
    }
    else if (command->Equals("SEND_VIDEO", StringComparison::OrdinalIgnoreCase))
    {
        Console::WriteLine(command);
        // Nhận kích thước video
        array<Byte> ^ sizeBuffer = gcnew array<Byte>(4);
        int bytesRead = stream->Read(sizeBuffer, 0, sizeBuffer->Length);
        if (bytesRead != sizeBuffer->Length)
        {
            Console::WriteLine("Failed to receive video size.");
            return;
        }

        int videoSize = BitConverter::ToInt32(sizeBuffer, 0);
        if (videoSize <= 0)
        {
            Console::WriteLine("Invalid video size received.");
            return;
        }
        Console::WriteLine("Video size: " + videoSize);

        // Nhận dữ liệu video
        array<Byte> ^ videoData = gcnew array<Byte>(videoSize);
        int totalBytesRead = 0;
        while (totalBytesRead < videoSize)
        {
            int read = stream->Read(videoData, totalBytesRead, videoSize - totalBytesRead);
            if (read <= 0)
                break;
            totalBytesRead += read;
        }

        // Lưu video
        String ^ videoPath = "Video_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".mp4";
        File::WriteAllBytes(videoPath, videoData);
        Console::WriteLine("Video saved as: " + videoPath);

        // Gửi email
        String ^ subject = "Video from Server";
        String ^ body = "Here is the Video you requested.";
        SendEmail(recipientEmail, subject, body, videoPath);
    }
    else if (command->Contains("GET_FILE"))
    {
        // Bước 1: Gửi yêu cầu nhận file đến server
        Console::WriteLine("Requesting file from server...");

        // Bước 2: Nhận kích thước của file từ server
        array<Byte> ^ sizeBuffer = gcnew array<Byte>(16);
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
        array<Byte> ^ fileData = gcnew array<Byte>(fileSize);
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
        String ^ filePath = "received_file_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".dat"; // Đặt tên cho file
        File::WriteAllBytes(filePath, fileData);
        Console::WriteLine("File received and saved as: " + filePath);

        // Gửi email thông báo cho người dùng với file đã nhận làm tệp đính kèm
        String ^ subject = "File Received from Server";
        String ^ body = "Here is the file you requested.";
        SendEmail(recipientEmail, subject, body, filePath);
    }
    else
    {
        array<Byte> ^ buffer = gcnew array<Byte>(65536);         // Tạo bộ đệm để lưu dữ liệu nhận về
        int bytesRead = stream->Read(buffer, 0, buffer->Length); // Đọc dữ liệu từ server

        if (bytesRead > 0)
        {
            // Chuyển dữ liệu nhận được sang chuỗi
            String ^ response = Encoding::UTF8->GetString(buffer, 0, bytesRead);
            Console::WriteLine("Response from server:\r\n{0}", response);

            // Lưu dữ liệu nhận được vào file txt
            String ^ filePath = "response_" + DateTime::Now.ToString("yyyyMMdd_HHmmss") + ".txt";
            File::WriteAllText(filePath, response);

            // Gửi file txt qua email
            String ^ subject = "Response from Server";
            String ^ body = "Here is the response from the server saved in a text file.";
            SendEmail(recipientEmail, subject, body, filePath);

            Console::WriteLine("Response saved to file and sent via email.");
        }
        else
        {
            Console::WriteLine("No response from server.");
        }
    }
}

int main(array<System::String ^> ^ args)
{
    try
    {
        // Lưu lại thời gian khi ứng dụng khởi động
        DateTime startTime = DateTime::Now;

        // Địa chỉ email của người gửi cụ thể cần lọc (có thể để trống nếu không cần lọc theo người gửi)
        String ^ specificSender = "notfound567404@gmail.com";

        // Thư mục tạm cho email đã xử lý
        String ^ processedEmailsFile = Path::Combine(Directory::GetCurrentDirectory(), "processed_emails.txt");

        // Cấu hình máy chủ Gmail IMAP
        MailServer ^ oServer = gcnew MailServer("imap.gmail.com",
                                                "huynhtrungkiet09032005@gmail.com", // Địa chỉ email của bạn
                                                "ifztigzwrspjwhch",                 // Mật khẩu ứng dụng
                                                ServerProtocol::Imap4);             // Giao thức IMAP

        oServer->SSLConnection = true; // Sử dụng kết nối SSL/TLS
        oServer->Port = 993;           // Cổng SSL/TLS IMAP

        Console::WriteLine("Connecting to IMAP server...");

        // Tạo đối tượng MailClient
        MailClient ^ oClient = gcnew MailClient("TryIt");

        while (true) // Liên tục kiểm tra email mới
        {
            try
            {
                oClient->Connect(oServer);
                Console::WriteLine("Connected to IMAP server");
                array<Imap4Folder ^> ^ folders = oClient->GetFolders();
                Imap4Folder ^ inboxFolder = FindInboxFolder(folders);

                if (inboxFolder != nullptr)
                {
                    oClient->SelectFolder(inboxFolder);
                    array<MailInfo ^> ^ infos = oClient->GetMailInfos();
                    Console::WriteLine("Total {0} email(s)\r\n", infos->Length);

                    if (infos->Length > 0)
                    {
                        for (int i = infos->Length - 1; i >= 0; i--)
                        {
                            MailInfo ^ mailInfo = infos[i];
                            String ^ messageId = mailInfo->UIDL; // Sử dụng UIDL để phân biệt email

                            if (IsEmailProcessed(messageId, processedEmailsFile))
                            {
                                Console::WriteLine("Email already processed. Skipping...");
                                continue; // Bỏ qua email này
                            }

                            // Lấy nội dung email
                            EAGetMail::Mail ^ oMail = oClient->GetMail(mailInfo);

                            // Kiểm tra thời gian gửi email
                            DateTime sentTime = oMail->ReceivedDate;

                            // Nếu email được gửi trước thời gian ứng dụng chạy, thoát khỏi vòng lặp
                            if (sentTime < startTime)
                            {
                                Console::WriteLine("Found an email sent before application started. Stopping...");
                                break;
                            }

                            Console::WriteLine("Processing email sent at: {0}", sentTime);
                    

                            // Kiểm tra người gửi có khớp với địa chỉ cụ thể không (nếu cần thiết)
                            if (specificSender->Length == 0 || IsEmailFromSpecificSender(oMail, specificSender))
                            {
                                Console::WriteLine("From: {0}", oMail->From->ToString());
                                Console::WriteLine("Subject: {0}\r\n", oMail->Subject);
                                Console::WriteLine("Body: {0}\r\n", oMail->TextBody); // In ra nội dung email

                                // Gửi lệnh từ nội dung email đến server điều khiển
                                String ^ ip_add = ExtractIpFromEmailBody(oMail->TextBody->Trim());
                                String ^ command = ExtractCommandFromEmailBody(oMail->TextBody->Trim());

                                // Thực thi lệnh qua socket
                                try
                                {
                                    TcpClient ^ client = gcnew TcpClient();
                                    client->Connect(ip_add, 12345); // Kết nối đến server
                                    NetworkStream ^ stream = client->GetStream();

                                    // Gửi lệnh đến server
                                    array<Byte> ^ data = Encoding::UTF8->GetBytes(command);
                                    stream->Write(data, 0, data->Length);
                                    stream->Flush(); // Đảm bảo dữ liệu đã được gửi đi

                                    // Xử lý phản hồi từ server
                                    handleResponse(command, stream, specificSender);
                              

                                    // Đóng kết nối
                                    stream->Close();
                                    client->Close();
                                }
                                catch (Exception ^ ex)
                                {
                                    Console::WriteLine("Error connecting to server at IP {0}: {1}", ip_add, ex->Message);

                                    // Gửi email thông báo lỗi lại cho người gửi
                                    String ^ subject = "Error: Failed to Connect to Server";
                                    String ^ body = "The IP address provided in your email (" + ip_add +
                                                    ") could not be connected to. Please verify the IP address and try again.\r\n\r\nError Details:\r\n" + ex->Message;
                                    SendEmail(oMail->From->Address, subject, body, nullptr);
                                }

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
            catch (Exception ^ e)
            {
                Console::WriteLine("Error: {0}", e->Message);
            }

            // Sleep for 60 seconds before checking again
            Thread::Sleep(60000);  // 60 seconds
        }
    }
    catch (Exception ^ ep)
    {
        Console::WriteLine("Error: {0}", ep->Message);
    }

    return 0;
}
