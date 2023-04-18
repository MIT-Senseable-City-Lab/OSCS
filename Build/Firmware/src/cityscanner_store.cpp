#include "cityscanner_store.h"

CityStore *CityStore::_instance = nullptr;

CityStore::CityStore() {}

int CityStore::init()
{
    deviceID = System.deviceID();
    records = RECORDS_PER_FILE;
    Serial.println("\nInitializing SD card...");
    if (!SD.begin(chipSelect))
    {
        Serial.println("SD initialization failed");
    }
    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    // list all files in the card with date and size
    SD.ls(LS_R | LS_DATE | LS_SIZE);
    Serial.println(F("Card successfully initialized."));
    Serial.flush();
    if (!SD.mkdir("queue"))
    {
        Serial.println("queue folder already exists or cannot be created!");
    }
    if (!SD.mkdir("done"))
    {
        Serial.println("done folder already exists or cannot be created!");
    }
    if (SD.exists("queue"))
    {
        Serial.println("queue folder exists");
    }
    if (SD.exists("done"))
    {
        Serial.println("done folder exists");
    }
    // create active data log
    switch_logfile();
    return 1;
}

int CityStore::stop()
{
  activeFile.flush();
  SD.end();
  return 1;
}

int CityStore::switch_logfile()
{
  // running for the first time
  if (!SD.exists("active.csv"))
  {
    activeFile = SD.open("active.csv", O_WRITE | O_CREAT | O_APPEND);
    if (activeFile)
        Serial.println("active.csv created");    
    cnt = 1;
    return 1;
  }
  else if (!activeFile)
  {
    activeFile = SD.open("active.csv", O_WRITE | O_CREAT | O_APPEND);
    if (activeFile)
      Serial.println("active.csv is opened!");
    cnt = 1;
    return 1;
  }

  if (activeFile)
  {
    Serial.print("Active file size: ");
    Serial.println(activeFile.size());
    activeFile.close();
  }

  // rename active file and move it to queue folder
  String fileName = String::format("%02d%02d%02d%02d", Time.month(), Time.day(), Time.hour(), Time.minute());
  fileName = "queue/" + fileName;
  fileName = fileName + ".csv";
  File newFile = SD.open(fileName, O_WRITE | O_CREAT | O_APPEND);
  if (newFile)
  {
    activeFile = SD.open("active.csv", O_READ);
    size_t n;
    uint8_t buf[1000];
    while ((n = activeFile.read(buf, sizeof(buf))) > 0)
    {
      newFile.write(buf, n);
      newFile.flush();  //flushing makes sure any bytes written to the file are physically saved to the SD card.
    }
    newFile.close();
    activeFile.close();
    Serial.println("Rename successfull");
    SD.remove(activeFile.name());
  }
  else
    Serial.println("Failed to rename");
  // create new active file
  activeFile = SD.open("active.csv", O_WRITE | O_CREAT | O_APPEND);
  if (!activeFile) 
    Serial.println("opening new file failed!");
  else
    Serial.println("File switch successfull : " + fileName);
  cnt = 1;
  return 1;
}

void CityStore::logData(int broadcastType, int payloadType, String data)
{
  //String output = String::format("%d,%s,%d,%s,%s", payloadType, deviceID.c_str(), (int)Time.now(), LocationService::instance().getGPSdata().c_str(), data.c_str());
  String output = String::format("%d,%s,%s,%s,%s", payloadType, deviceID.c_str(), LocationService::instance().getEpochTime().c_str(), LocationService::instance().getGPSdata().c_str(), data.c_str());
  writeData(output);

  switch (broadcastType)
  {
  case BROADCAST_IMMEDIATE:
    if(Particle.connected())
      Particle.publish("DAT4",output);
    break;
  default:
    break;
  }
}

void CityStore::writeData(String data)
{
  Serial.print("Record to file:"); Serial.println(data);
  activeFile.println(data);
  activeFile.flush();
  cnt += 1;
  Serial.print("N. records written to file : "); Serial.println(cnt);
  if (cnt % records == 0) //keep
  {
    Serial.println("SWITCHING FILE"); 
    switch_logfile();
    cnt = 1;
  }
}

// Numbers of files in the queue folder to be dumped via tcp or ALL_FILES
bool CityStore::dumpData(int files_to_dump)
{
    if (files_to_dump == ALL_FILES) {
        files_to_dump = countFilesInQueue();
    }
    else if (files_to_dump > countFilesInQueue()){
        files_to_dump = countFilesInQueue();
    }
    
    Serial.println("Started dumping data"); //here for debugging purposes
    File queueFolder = SD.open("/queue", O_READ);
    if (!queueFolder) {
        Serial.println("queue cannot be opened!");
        return false;
    }

    Serial.print(files_to_dump);
    Serial.println(" are going to be transmitted");
    int i = 0;

    if (client.connect(TCP_ENDPOINT, 1024) | TCP_GHOSTWRITE)
    {
        Serial.println("Connection to endpoint established");
        File file;
        for (i = 0; i < files_to_dump; i++)
         {
            file = queueFolder.openNextFile(O_READ);
            if (!file){
                Serial.println("File retrieve from queue failed (no file to broadcast)!");
                return false;
            }
            String filename = "queue/" + String(file.name());
            Serial.print("Dumping file : ");
            Serial.println(filename);
            Serial.print("Size: ");
            int file_size = file.size();
            Serial.println(file_size);
            if (file.size() > 0)
            {
                char *body;
                body = (char *)malloc(file_size);
                file.read(body, file_size);
                if(TCP_GHOSTWRITE)
                {
                    Serial.println("Sending the following data over TCP"); Serial.println(body);
                }
                if(!TCP_GHOSTWRITE)
                {
                // write TCP body
                int body_resp = client.write(reinterpret_cast<const uint8_t *>(body), file_size);
                Serial.print("TCP body write result: "); Serial.println(body_resp);
                client.flush();
                }
                free(body);
            }
            file.close(); // close to open again later to read from the beginning of file;
            // move file from queue to done folder
            String newFileName = "done/" + String(file.name());
            File newFile = SD.open(newFileName, O_WRITE | O_CREAT | O_APPEND);
            if (newFile)
            {
                size_t n;
                uint8_t buf[1000];
                file = SD.open(filename, O_READ);
                while ((n = file.read(buf, sizeof(buf))) > 0)
                {
                    newFile.write(buf, n);
                    newFile.flush(); //flushing makes sure any bytes written to the file are physically saved to the SD card.
                }
                newFile.close();
                file.close();
                SD.remove(filename);
            }
            else{
                Serial.println("File could not be moved to done folder!");
                file.close();
            }
        }
        client.stop();
        return true;
    }
    else{
        Serial.println("The connection cannot be established");
        return false;
    }
}

int CityStore::countFilesInQueue()
{
  File queueFolder;
  File file;
  int k = 0;
  queueFolder = SD.open("/queue", O_READ);
  if (!queueFolder){
    Serial.println("queue cannot be opened!");
    return false;
  }
  file = queueFolder.openNextFile(O_READ);
  while (file) {
    k++;
    file.close();
    file = queueFolder.openNextFile(O_READ);
  }
  queueFolder.close();
  return k;
}

bool CityStore::deleteAll(bool removeDirs)
{
  delFiles("/queue");
  delFiles("/done");

  if (removeDirs)
  {
    delFiles("/");
    SD.rmdir("/queue");
    SD.rmdir("/done");
  }
  Serial.println("All removed!");
  return true;
}

void CityStore::delFiles(const char *folder_name)
{
  File file;
  File folder;
  folder = SD.open(folder_name, O_READ);
  if (!folder)
  {
    Serial.print(folder_name);
    Serial.println(" unable to open");
  }
  file = folder.openNextFile(O_READ);
  while (file) 
  {
    String filename;
    if (strcmp(folder.name(), "/") == 0){
      filename = String(file.name());
    }
    else{
      filename = String(folder.name()) + "/" + String(file.name());
    }
    Serial.print(filename);
    Serial.print(" ---> ");
    Serial.println(file.size());
    bool isDir = file.isDirectory();
    file.close();
    if (!isDir)
    {
      if (!SD.remove(filename))
        Serial.println("Error file remove!");
    }
    file = folder.openNextFile(O_READ);
  }
  folder.close();
}

void CityStore::reInit()
{
  Serial.println("Re-intializing the sd-card");
  Serial.println("Deleting all files...");
  activeFile.close();
  deleteAll(1);
  Serial.println("All files deleted");
  if (!SD.mkdir("queue")) {
    Serial.println("queue folder already exists or cannot be created!");
  }
  else{
    Serial.println("Queue folder created");
  }
  if (!SD.mkdir("done")){
    Serial.println("done folder already exists or cannot be created!");
  }
  else{
    Serial.println("Done folder created");
  }
  Serial.println("Sd re-initialized");
  switch_logfile();
}
