#include "SPI.h"
#include "SD.h"


const uint8_t chipSelect = SS;
File activeFile;

void setup(){
Serial.println("\nInitializing SD card...");
  if (!SD.begin(chipSelect))
  {
    Serial.print(F(
        "\nSD initialization failed.\n"
        "Do not reformat the card!\n"
        "Is the card correctly inserted?\n"
        "Is chipSelect set to the correct value?\n"
        "Does another SPI device need to be disabled?\n"
        "Is there a wiring/soldering problem?\n"));
  }

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  Serial.println("\nFiles found on the card (name, date and size in bytes): ");

  // list all files in the card with date and size
  SD.ls(LS_R | LS_DATE | LS_SIZE);

  Serial.println(F("Card successfully initialized."));
  Serial.flush();

  // Creating folders
  if (!SD.mkdir("queue"))
  {
    Serial.println("queue folder already exists or cannot be created!");
  }
  if (!SD.mkdir("done"))
  {
    Serial.println("done folder already exists or cannot be created!");
  }
  // Just making sure that both folders are created
  if (SD.exists("queue"))
  {
    Serial.println("queue folder exists");
  }
  if (SD.exists("done"))
  {
    Serial.println("done folder exists");
  }
}

void loop(){

    activeFile = SD.open("active.log", O_WRITE | O_CREAT | O_APPEND);
    String output = "output";
    activeFile.println(output);
    activeFile.flush();
    SD.remove(activeFile.name());
    activeFile.flush();
    activeFile.close();


}