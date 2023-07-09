
void setup_webserver (){

    webserver.on("/", HTTP_GET, handle_home);
    webserver.on("/reset", HTTP_GET, handle_reset);
    webserver.on("/debug", HTTP_GET, handle_debug);
    webserver.on("/upload", HTTP_POST, [](){handle_redirect();}, handle_upload);
    webserver.on("/delete", HTTP_POST, [](){handle_delete(); handle_redirect();});
    webserver.begin();

}

//https://arduino.stackexchange.com/questions/61852/arduino-ide-esp32-webserver-how-to-redirect-to-root-after-click-on-button
void handle_redirect(){
    webserver.sendHeader("Location", "/", true);
    webserver.send(302, "text/plain", "");
}

void handle_home() {
    Serial.println("Handle Home");
    webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webserver.send(200, "text/html", HTML_BODY);
    int cnt = 0;
    index_t item;
    bool restart = true;
    int flag;
    while(true){
        flag = traverse_index(&item, restart, true);
        if(flag == -1) break;
        restart = false;
        String html = "";
        html += "<tr>";
        html += "<td>" + String(cnt) + "</td>";
        html += "<td>" + String((const char *)item.name) + "</td>";
        html += "<td>" + String(item.size) + "</td>";
        html += "<td><form method=post action=/delete><button name=button value=" + String((const char *)item.name) + ">Delete</button></form></td>";
        html += "</tr>";
        cnt++;
        webserver.sendContent(html);
        if(flag == 1) break;
    }
    webserver.sendContent("</table></body></html>");
    webserver.sendContent("");
}

void handle_reset() {
    SD.remove(INDEX_FILE);
    File root = SD.open("/");
    if (root) {
        root.rewindDirectory();
        File file = root.openNextFile();
        while(file){
            String name = String(file.name());
            if(name.endsWith(".jpg") || name.endsWith(".gif") || name.endsWith(".txt") || name.endsWith(".bmp")){
                Serial.println("Reset is deleting file: " + name);
                SD.remove("/" + name);
            }
            file = root.openNextFile();
        }
        root.close();
    } else {
        Serial.println("Error, couldn't open root directory");
    }
    handle_redirect();
}

void handle_debug() {
    Serial.println("DEBUG: printing all indexes:");
    for(int i = 0; i < INDEX_ENTRIES; i++){
        read_index(&item, i);
        print_index(&item);
    }
    handle_redirect();
}

File file2upload;
int filesize = 0;

void handle_upload(){
    HTTPUpload& upload = webserver.upload();
    int size, bytes, final;
    size = upload.totalSize;
    bytes = upload.currentSize;
    String filename = "/" + String(upload.filename.c_str());
    Serial.printf("Handling upload: file=%s, size=%d, bytes=%d\n\r", filename, filesize, bytes);
    if(upload.status == UPLOAD_FILE_START){
        file2upload = SD.open(filename, "w");    
        filesize = 0;
    } else if(upload.status == UPLOAD_FILE_WRITE){
        file2upload.write(upload.buf, bytes);
        filesize += bytes;
    } else if (upload.status == UPLOAD_FILE_END){
        file2upload.close();
        fsmutex.lock();
        add_index(upload.filename.c_str(), filesize);
        fsmutex.unlock();
    }
}

void handle_delete() {
    String path = webserver.arg(0);
    Serial.printf("Handling delete: file=%s\n\r", path);
    delete_queue.push_back(path);
    fsmutex.lock(); 
    delete_index(path.c_str());
    fsmutex.unlock();
}

