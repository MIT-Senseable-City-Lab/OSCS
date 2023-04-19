#ifndef __ASSETTRACKERRK_H
#define __ASSETTRACKERRK_H

#include "Particle.h"

#include "TinyGPS++.h"
#include "LegacyAdapter.h"
#include "UbloxGPS.h"


/**
 * @brief Base functionality for GNSS functionality using TinyGPS
 */
class AssetTrackerBase : public LegacyAdapter {
public:
	AssetTrackerBase();
	virtual ~AssetTrackerBase();
	void begin(); 

	/**
	 * @brief Updates the GPS. Must be called from loop() as often as possible, typically every loop.
	 *
	 * Only call this when not using threaded mode.
	 */
	void updateGPS(void);

	/**
	 * @brief Enable GPS threaded mode
	 *
	 * In threaded mode, the serial port is read from a separate thread so you'll be less likely to
	 * lose data if loop() is blocked for any reason
	 */
	void startThreadedMode();


	void enterSleep();

	void wake();

	/**
	 * @brief Sends a u-blox GPS command.
	 *
	 * @param cmd The pointer to a uint8_t array of bytes to send
	 *
	 * @param len The length of the command.
	 */
	void sendCommand(const uint8_t *cmd, size_t len);

	/**
	 * @brief Sets the external decoder function
	 * 
	 * @param fn function set. Replaces any existing function. Set to 0 to remove.
	 * 
	 * This is used by the UbloxGPS module to hook into the GPS decoding loop. You
	 * probably won't have to use this directly. Other brands of GPS that decode
	 * things that TinyGPS++ can't might also use this to hook in.
	 */
	void setExternalDecoder(std::function<bool(char)> fn) { externalDecoder = fn; };

	/**
	 * @brief Set a function to be called during the threaded mode loop
	 */
	void setThreadCallback(std::function<void(void)> fn) { threadCallbacks.push_back(fn); };

	/**
	 * @brief Set a function to be called during the when a full sentence is received
	 */
	void setSentenceCallback(std::function<void(void)> fn) { sentenceCallbacks.push_back(fn); };

	/**
	 * @brief Override the default serial port used to connect to the GPS. Default is Serial1.
	 */
	AssetTrackerBase &withSerialPort(USARTSerial &port);

	/**
	 * @brief Use I2C (DDC) instead of serial.
	 *
	 * @param wire The I2C interface to use. This is optional, and if not set, Wire (the standard I2C interface) is used.
	 * On some devices (Electron, Argon, and Xenon), there is an optional Wire1.
	 *
	 * @param addr The I2C address to use. This is optional, and the default is 0x42.
	 * The address can be reprogrammed in software on the u-blox GPS, but 0x42 is the default.
	 */
	AssetTrackerBase &withI2C(TwoWire &wire = Wire, uint8_t addr = 0x42);

	AssetTrackerBase &withGNSSExtInt(pin_t extIntPin) { this->extIntPin = extIntPin; return *this; };

	/**
	 * @brief Gets the TinyGPS++ object so you can access its methods directly
	 */
	TinyGPSPlus *getTinyGPSPlus() { return &gps; };

	/**
	 * @brief Lock the mutex. Used to prevent multiple threads from writing to the GPS at the same time
	 */
	void lock();

	/**
	 * @brief Lock the mutex. Used to prevent multiple threads from writing to the GPS at the same time
	 */
	void unlock();

	/**
	 * @brief Get the singleton instance of this class
	 */
	static AssetTrackerBase *getInstance() { return instance; }; 


protected:
	uint16_t wireReadBytesAvailable();
	int wireReadBytes(uint8_t *buf, size_t len);

	void threadFunction();
	static void threadFunctionStatic(void *param);

	TinyGPSPlus gps;
	bool useWire = false;
	TwoWire &wire = Wire;
	uint8_t wireAddr = 0x42;
	USARTSerial &serialPort = Serial1;
	Thread *thread = NULL;
	std::function<bool(char)> externalDecoder = 0;
	std::vector<std::function<void()>> threadCallbacks;
	std::vector<std::function<void()>> sentenceCallbacks;
	pin_t extIntPin = PIN_INVALID;
	os_mutex_t mutex = 0;
	static AssetTrackerBase *instance;
};

/**
 * @brief Class used for devices that use an antenna switch like the Electron AssetTracker v2
 */
class AssetTrackerAntennaSwitch {
public:
	AssetTrackerAntennaSwitch();
	virtual ~AssetTrackerAntennaSwitch();


protected:
};



/**
 * @brief Compatible replacement for the official Particle Electron AssetTracker library.
 *
 * This is an almost drop-in replacement based on my LIS3DH driver and TinyGPS++.
 * 
 * There are other classes that you can use to mix-and match features if you want to
 * used a custom design with different peripherals (see AssetTrackerFeather6).
 *
 * Note that many of the backward compatibility methods are in the LegacyAdapter class
 * so make sure you check there as well.
 */
class AssetTracker : public AssetTrackerBase {

public:
	/**
	 * @brief Construct an AssetTracker object. Normally this is a global variable.
	 */
	AssetTracker();

	/**
	 * @brief Destructor
	 */
	virtual ~AssetTracker();

	/**
	 * @brief Initialize the LIS3DH accelerometer. Optional.
	 */
	void begin(void);


	/**
	 * @brief Turns the GPS on.
	 *
	 * By default, it's off on reset. Call gpsOn() to turn it on.
	 */
	void gpsOn(void);

	/**
	 * @brief Turns the GPS off.
	 */
	void gpsOff(void);

	/**
	 * @brief Select the internal antenna
	 *
	 * On the AssetTracker v2 only (or other u-blox GPS), sets the antenna to internal. Internal is the default
	 * and is reset every time the GPS is powered up.
	 *
	 * On the AssetTracker v1 (or other PA6H GPS), the antenna is auto-switching and this call is ignored.
	 */
	bool antennaInternal();


	/**
	 * @brief Select the external antenna
	 *
	 * On the AssetTracker v2 only (or other u-blox GPS), sets the antenna to external. Internal is the default
	 * and is reset every time the GPS is powered up.
	 *
	 * On the AssetTracker v1 (or other PA6H GPS), the antenna is auto-switching and this call is ignored.
	 */
	bool antennaExternal();

	/**
	 * @brief Not supported. The data is not available from TinyGPS++
	 *
	 * An empty string is always returned.
	 */
	char *preNMEA(void);

private:
	bool fakevariable;
};

#endif /* __ASSETTRACKERRK_H */
