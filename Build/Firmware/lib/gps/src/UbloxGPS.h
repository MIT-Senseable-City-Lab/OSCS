#ifndef __UBLOXGPS_H
#define __UBLOXGPS_H

#include "Particle.h"

#include "google-maps-device-locator.h" // Only used if UbloxAssistNow is used

#include <deque>
#include <vector>

//class UbloxCommandBase; // Foreward declaration



/**
 * @brief Class for reading or writing a u-blox command.
 * 
 * You normally won't use this class directly, but will do so indirectly two ways:
 * 
 * - Using UbloxCommand<> a templated version with a buffer to store a commmand you're building and
 * will evenutally send to the modem
 * - Using Ublox, the class for managing the modem, which subclasses this for received messages
 * and acknowledgements.
 */
class UbloxCommandBase {
public:
	/**
	 * @brief Construct a UbloxCommandBase object. Normally you'd use UbloxCommand instead, which handles
	 * the buffer management for you.
	 *
	 * @param buffer Pointer a buffer. Must remain valid for the life of this object. Must not be NULL.
	 *
	 * @param bufferSize Size of the buffer, a minimum of HEADER_PLUS_CRC_LEN (8) bytes. You will overwrite
	 * memory if the buffer is smaller than that.
	 */
	UbloxCommandBase(uint8_t *buffer, size_t bufferSize);

	/**
	 * @brief Construct a UbloxCommandBase object with a buffer allocated on the heap.
	 *
	 * @param bufferSize Size of the buffer, a minimum of HEADER_PLUS_CRC_LEN (8) bytes. You will overwrite
	 * memory if the buffer is smaller than that.
	 */
	UbloxCommandBase(size_t bufferSize);

	/**
	 * @brief Destructor
	 */
	virtual ~UbloxCommandBase();

	/**
	 * @brief Decode a single character. 
	 * 
	 * This is called after reading data from the GPS by serial or I2C.
	 */
	bool decode(char ch);

	/**
	 * @brief Used internally to discard invalid data. You probably won't need to call this.
	 */
	void discardToNextSync1();

	/**
	 * @brief When preparing a command to send, updates the checksum, sync, and length bytes
	 *
	 * The length is generated from the payloadLen member variable, which should be correct
	 * when using the set or append methods, or after manually setting the length.
	 */
	void updateChecksum();

	/**
	 * @brief Calculates the checksum based on the data in the buffer
	 *
	 * The checksum is calculated not including the sync bytes or the CRC itself. This
	 * method returns the checksum values as parameters, so you can compare them.
	 *
	 * The updateChecksum() method is typically used to calculate it for sending.
	 */
	void calculateChecksum(uint8_t &ckA, uint8_t &ckB) const;

	/**
	 * @brief Gets the message class of the current command
	 * 
	 * This can be used from a message handler to find the message class of the message that
	 * is being handled.
	 */
	uint8_t getMsgClass() const { return buffer[CLASS_OFFSET]; };

	/**
	 * @brief Gets the message ID of the current command
	 * 
	 * This can be used from a message handler to find the message id of the message that
	 * is being handled.
	 */
	uint8_t getMsgId() const { return buffer[ID_OFFSET]; };


	/**
	 * @brief When sending a message, sets the message class and ID
	 * 
	 * @param msgClass The class of the message
	 * @param msgId The ID of the message
	 */
	void setClassId(uint8_t msgClass, uint8_t msgId);

	/**
	 * @brief Gets the payload length, the number of bytes available from getData()
	 */
	size_t getPayloadLen() const { return payloadLen; };

	/**
	 * @brief Get a const pointer to the data payload
	 * 
	 * There are other methods to copy the data, and read out values like U2, R4, etc..
	 */
	const uint8_t *getData() const { return &buffer[DATA_OFFSET]; };

	/**
	 * @brief Copies data from the buffer
	 *
	 * @param offset The offset to being reading from. This is relative to the data payload, so 0
	 * is the first byte of payload data (not the sync byte)
	 *
	 * @param data Buffer to copy data into
	 *
	 * @param dataLen The length of the data. This must be <= (payloadLen - offset)
	 *
	 * @return true if the data could be retrieved or false if it could not
	 *
	 * Will only return true if all of the data can be returned. If offset + dataLen > the payloadLen
	 * the false will be returned and no data will be returned.
	 */
	bool getData(size_t offset, void *data, size_t dataLen) const;

	/**
	 * @brief Set data in the buffer
	 *
	 * @param offset The offset to write to. This is relative to the data payload, so 0
	 * is the first byte of payload data (not the sync byte)
	 *
	 * @param data Pointer to the data to copy from.
	 *
	 * @param dataLen The length of the data in bytes.
	 *
	 * @return true if it was copied or false if not
	 *
	 * The data will be copied only if it will fit in the buffer entirely. The data will never
	 * be truncated. The payloadLen will be increased if necessary so the data fits within
	 * the payloadLen.
	 */
	bool setData(size_t offset, const void *data, size_t dataLen);

	/**
	 * @brief Appends data into the buffer at the current payloadLen
	 *
	 * @param data Pointer to the data to copy from.
	 *
	 * @param dataLen The length of the data in bytes.
	 *
	 * @return true if it was copied or false if not
	 *
	 * The data will be copied only if it will fit in the buffer entirely. The data will never
	 * be truncated. The payloadLen will be increased by dataLen bytes.
	 */
	bool appendData(const void *data, size_t dataLen);

	/**
	 * @brief Works like appendData(), but fill with a value instead
	 * 
	 * @param value The uint8_t value to set. Often 0 to initialize to zero.
	 * 
	 * @param dataLen The number of bytes to fill with value.
	 * 
	 * Handy for pre-creating zero-filled command buffers that are zeroed out.
	 */
	bool fillData(uint8_t value, size_t dataLen);

	/**
	 * @brief Get a uint8_t value at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	uint8_t getU1(size_t offset) const;

	/**
	 * @brief Set a uint8_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */
	bool setU1(size_t offset, uint8_t value) { return setData(offset, &value, sizeof(value)); };

	/**
	 * @brief Append a uint8_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit.
	 */
	bool appendU1(uint8_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a int8_t value (signed) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	int8_t getI1(size_t offset) const { return (int8_t)getU1(offset); };

	/**
	 * @brief Set a int8_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */
	bool setI1(size_t offset, int8_t value) { return setData(offset, &value, sizeof(value)); };

	/**
	 * @brief Append a int8_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit.
	 */
	bool appendI1(int8_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a uint16_t value (little endian) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	uint16_t getU2(size_t offset) const;
	
	/**
	 * @brief Set a uint16_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */
	bool setU2(size_t offset, uint16_t value) { return setData(offset, &value, sizeof(value)); };
	/**
	 * @brief Append a uint16_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */
	bool appendU2(uint16_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a int16_t value (little endian, signed) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	int16_t getI2(size_t offset) const { return (int16_t)getU2(offset); };

	/**
	 * @brief Set an int16_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */
	bool setI2(size_t offset, int16_t value) { return setData(offset, &value, sizeof(value)); };
	
	/**
	 * @brief Append an int16_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */
	bool appendI2(int16_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a uint32_t value (little endian, 4 bytes) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	uint32_t getU4(size_t offset) const;

	/**
	 * @brief Set a uint32_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */	
	bool setU4(size_t offset, uint32_t value) { return setData(offset, &value, sizeof(value)); };

	/**
	 * @brief Append a uint32_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */	
	bool appendU4(uint32_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a int32_t value (little endian, 4 bytes, signed) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	int32_t getI4(size_t offset) const { return (int32_t)getU4(offset); };

	/**
	 * @brief Set an int16_t value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */	
	bool setI4(size_t offset, int32_t value) { return setData(offset, &value, sizeof(value)); };
	
	/**
	 * @brief Append a int32_t value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */	
	bool appendI4(int32_t value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a float value (4 bytes) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	float getR4(size_t offset) const;

	/**
	 * @brief Set an float (4 byte floating point) value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */	
	bool setR4(size_t offset, float value) { return setData(offset, &value, sizeof(value)); };
	/**
	 * @brief Append a float (4 byte floating point) value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */	
	bool appendR4(float value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a double value (8 bytes) at offset 
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 */
	double getR8(size_t offset) const;

	/**
	 * @brief Set an double (8 byte floating point) value at offset in the payload
	 * 
	 * @param offset The offset relative to the payload, so 0 is the first byte of the payload
	 * 
	 * @param value The value to set
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 * If the new data is larger than the current payloadLen, then the payload size is increased
	 * so it will contain the newly set data.
	 */	
	bool setR8(size_t offset, double value) { return setData(offset, &value, sizeof(value)); };

	/**
	 * @brief Append a double (8 byte floating point) value to the current end of the payload
	 * 
	 * @param value The value to append
	 * 
	 * The data must fit in the underlying buffer or false will be returned if it can't fit
	 * in its entirety. A partial value will never be copied.
	 */	bool appendR8(double value) { return appendData(&value, sizeof(value)); };

	/**
	 * @brief Get a pointer to the buffer, typically to send data
	 *
	 * This is a pointer to the first sync byte.
	 */
	const uint8_t *getBuffer() const { return buffer; }

	/**
	 * @brief Get the length of the data, typically to send it.
	 *
	 * It's the length from the first sync byte through the last CRC byte, including the data payload.
	 */
	size_t getSendLength() const { return HEADER_PLUS_CRC_LEN + payloadLen; };

	/**
	 * @brief Set the deleteBuffer flag for this object
	 */
	UbloxCommandBase &withDeleteBuffer(bool value = true) { deleteBuffer = value; return *this; };

	/**
	 * @brief Make a modifyable clone of this object on the heap
	 */
	UbloxCommandBase *clone();

	static const uint8_t CLASS_UBX_ACK = 0x05;			// 
	static const uint8_t   MSG_UBX_ACK_ACK = 0x01;		// 
	static const uint8_t   MSG_UBX_ACK_NACK = 0x00;		// 

	static const uint8_t CLASS_UBX_CFG = 0x06;			// 

	static const uint8_t CLASS_UBX_ANY = 0xff;
	static const uint8_t MSG_UBX_ANY = 0xff;

protected:
	/**
	 * @brief u-blox parsing state constants
	 */
	enum class State {
		LOOKING_FOR_START,  	//!< Looking for the first sync byte (SYNC_1, 0xb5)
		LOOKING_FOR_LENGTH,  	//!< Waiting for DATA_OFFSET bytes to arrive so we can check the length
		LOOKING_FOR_MESSAGE		//!< Waiting for the rest of the message (HEADER_PLUS_CRC_LEN + payloadLen bytes)
	};
	static const uint8_t SYNC_1 = 0xb5;				//!< value of the first sync byte at buffer[0]
	static const uint8_t SYNC_2 = 0x62;				//!< value of the second sync byte at buffer[1]
	static const size_t CLASS_OFFSET = 2;			//!< offset in buffer where the message class is stored
	static const size_t ID_OFFSET = 3;				//!< offset in buffer where the message ID is stored
	static const size_t DATA_OFFSET = 6;			//!< offset in buffer where data begins (after sync, class, id, and length)
	static const size_t HEADER_PLUS_CRC_LEN = 8;	//!< size of the header data (6 bytes) plus CRC (2 bytes). Minimum size of a valid packet with 0 bytes of payload data.
	static const size_t CRC_START_OFFSET = 2;  		//!< offset in buffer where the CRC calculation starts (does not include sync bytes)
	static const size_t CRC_HEADER_LEN = 4;  		//!< number of header bytes included in the CRC. The total CRC length is this plus payloadLen.

	uint8_t *buffer;						  		//!< Buffer to hold data (received data or data for composing a packet to send)
	size_t bufferSize;  							//!< Size of the buffer. Maximum payload is bufferSize - HEADER_PLUS_CRC_LEN.
	size_t bufferOffset = 0;  						//!< Current offset being written to in buffer when using encode()
	size_t payloadLen = 0;  						//!< Length of the data payload (0 = no data). This does not include the header or CRC.
	State state = State::LOOKING_FOR_START;  		//!< Current parsing state
	bool deleteBuffer = false;						//!< Delete buffer in the destructor
};


/**
 * @brief Class to hold a command to send or a command to decode
 *
 * It's a templated to specify the size of the buffer. The value is the size of the payload;
 * the actual buffer will be HEADER_PLUS_CRC_LEN more.
 */
template <size_t PAYLOAD_SIZE>
class UbloxCommand : public UbloxCommandBase {
public:
	/**
	 * @brief Constructs a command buffer of BUFFER_SIZE bytes.
	 *
	 * Must be a minimum of HEADER_PLUS_CRC_LEN bytes.
	 */
	explicit UbloxCommand() : UbloxCommandBase(staticBuffer, HEADER_PLUS_CRC_LEN + PAYLOAD_SIZE) {};

private:
	uint8_t staticBuffer[HEADER_PLUS_CRC_LEN + PAYLOAD_SIZE]; //!< The static buffer to hold the data
};

/**
 * @brief Structure holding information about a message handler
 */
typedef struct {
	enum class Reason : uint8_t {
		UNKNOWN = 0,// 0
		DATA,		// 1
		TIMEOUT,	// 2 Note: cmd will be null
		ACK,		// 3
		NACK,		// 4
		COMPLETE,	// 5 Note: cmd will be null
		UPDATE		// 6
	};

	/**
	 * @brief u-box message class (first of the two bytes)
	 * 
	 * 0xff = match anything.
	 */
	uint8_t classFilter;

	/**
	 * @brief u-blox message ID (second of the two bytes)
	 * 
	 * 0xff = match anything.
	 */
	uint8_t idFilter;

	/**
	 * @brief Function top call on match
	 */
	std::function<void(UbloxCommandBase *cmd, Reason reason)> handler;

	/**
	 * @brief Remove and delete this handler
	 * 
	 * If true, on returning from the handler, remove from the list of handlers and 
	 * delete the UbloxMessageHandler object. This is only done after the handler
	 * is called from callHandlers.
	 */
	bool removeAndDelete = false;

	/**
	 * @brief For CFG messages the original class ID for use in the ACK/NACK handler
	 */
	uint8_t origClassId = 0;

	/**
	 * @brief For CFG messages the original message ID for use in the ACK/NACK handler
	 */
	uint8_t origMsgId = 0;

	/**
	 * @brief Timeout time for ACK/NACK or response
	 */
	uint64_t timeout = 0;
} UbloxMessageHandler;

/**
 * @brief Command handler
 */
typedef std::function<void(UbloxCommandBase *, UbloxMessageHandler::Reason reason)> UbloxCommandCallback;


/**
 * @brief
 */
class UbloxSyncCommand {
public:
	UbloxSyncCommand();
	virtual ~UbloxSyncCommand();

	void completion(UbloxMessageHandler::Reason reason);

	UbloxMessageHandler::Reason blockUntilCompletion();

protected:
	os_mutex_t mutex;
	UbloxMessageHandler::Reason reason;
};

/**
 * @brief Class for implementing u-blox GPS support
 * 
 * You typically instantiate one of these as a global variable in your main application code
 * if you want to use the u-blox specific features. It is optional.
 * 
 * If you are using it, be sure to call the setup() and loop() methods from your application setup()
 * and loop()!
 */
class Ublox  {
public:
	/**
	 * @brief Constructor. You typically instantiate one of these as a global variable in your main
	 * application file.
	 */
	Ublox();

	/**
	 * @brief Destructor. As it is typically a global variable, you don't generally delete this.
	 */
	virtual ~Ublox();

	/**
	 * @brief Call from main application setup. Required!
	 */
	void setup();

	/**
	 * @brief Call from main application loop. Required!
	 */
	void loop();

	/**
	 * @brief Add a message handler
	 *
	 * @param handler A pointer to a filled in UbloxMessageHandler structure.
	 *
	 * Note the handler object  must remain valid until removeHandler is called on it, so you probably want
	 * to make it a global variable or allocated with new. The exception is if you have a blocking function
	 * call and add and remove the handler within the scope of the function, then you can allocate the
	 * object on the stack.
	 */
	void addHandler(UbloxMessageHandler *handler);

	/**
	 * @brief Remove a message handler
	 * 
	 * @param handler The handler to remove (same value you passed to addHandler)
	 * 
	 * Note this will not free handler as it can't know if it was allocated on the stack, as a 
	 * class member, global variable, or new. If you allocated the object with new, don't forget
	 * to delete it!
	 */
	// void removeHandler(UbloxMessageHandler *handler);

	/**
	 * @brief Returns true if cmd has a registered command handler
	 * 
	 * This eliminates the need to clone if the message will just be discarded.
	 */
	bool hasHandler(UbloxCommandBase *cmd);

	/**
	 * @brief Used internally from loop to call all of the message handlers that match this class and id
	 */
	void callHandlers();

	/**
	 * @brief Add a command to be handled from loop
	 * 
	 * @param cmd The command object. It must be a copy, returned from clone().
	 */
	void addCommandToHandle(UbloxCommandBase *cmd);

	/**
	 * @brief Asynchronous config (CFG 0x06)
	 */
	void configCommand(UbloxCommandBase *cmd, UbloxCommandCallback callback, unsigned long timeout = 5000);

	/**
	 * @brief Synchronous version of configCommand (CFG 0x06)
	 * 
	 * @return true if ACK is returned, false if NAK or timeout occurs
	 */
	bool configCommandSync(UbloxCommandBase *cmd, unsigned long timeout = 5000);

	void configGetSetValue(uint8_t msgClass, uint8_t msgId, UbloxCommandCallback callback, unsigned long timeout);

	/**
	 * @brief Get a command value from the u-blox modem
	 * 
	 * @param msgClass The class of the data to get
	 * 
	 * @param msgId The ID of the data to get
	 * 
	 * @param callback The function to call with the value
	 * 
	 * Get operations typically work by sending a command with a 0 byte payload. When the message
	 * is received (with the values), the callback is called
	 * 
	 * The callback has the prototype:
	 * 
	 *   void callback(UbloxCommandBase *cmd);
	 * 
	 * The cmd parameter allows the data to be examined. 
	 * 
	 * Note that when the AssetTracker is running in threaded mode, this will be called from the GPS
	 * reading thread so you should not do operations that block or are lengthy from the callback
	 * as it will affect GPS performance. 
	 * 
	 * Also note that the "cmd" passed to the callback only remains valid until the callback returns
	 * at which point the cmd object will be reused for the next response data.
	 */
	void getValue(uint8_t msgClass, uint8_t msgId, UbloxCommandCallback callback, unsigned long timeout = 5000);

	/**
	 * @brief Send a command top the GPS
	 * 
	 * @param cmd The command object to send
	 * 
	 * This class has some pre-made commands like setAntenna() but the sendCommand() can be used to
	 * send any arbitrary command.
	 * 
	 * This will send the data synchronously but there is no guarantee the GNSS received it or
	 * successfully processed it. "cmd" only needs to remain valid until the sendCommand method
	 * returns.
	 */
	void sendCommand(UbloxCommandBase *cmd);


	/**
	 * @brief Sets the antenna to external or internal on the AssetTracker V2.
	 */
	void setAntenna(bool external);
	
	/**
	 * @brief This function does not appear to work, but if it did, it would enable ACKs for aiding data
	 */
	void enableAckAiding();

	/**
	 * @brief Enable EXTINT Backup mode
	 * 
	 * This is used to put the GNSS in sleep ("Backup") mode using the EXTINT pin. If enabled, 
	 *   LOW = sleep/backup, HIGH = normal operation
	 * The GNSS can still wake up for scheduled operations.
	 * 
	 * If the Force-OFF (extintBackup) feature in UBX-CFG-PM2 is enabled, the receiver will enter Inactive 
	 * states for as long as the configured EXTINT pin is set to 'low' until the next wake up event. Any 
	 * wake-up event can wake up the receiver even while the EXTINT pin is set to 'low' (see Wake up). 
	 * However, if the pin stay at 'low' state, the receiver will only wake up for the time needed to 
	 * read the configuration pin settings then it will enter the Inactive state again.
	 */
	void enableExtIntBackup(bool enable, UbloxCommandCallback callback, unsigned long timeout = 5000);


	bool enableExtIntBackupSync(bool enable, unsigned long timeout = 5000);

	/**
	 * @brief Constants for whether to do a hot, warm, or cold restart using resetReceiver
	 */
	enum class StartType {
		HOT = 0,		//!< Restart with any current data
		WARM = 1,		//!< Restart without ephemeris, but with all other data
		COLD = 0xFFFF	//!< Restart clearing all saved data
	};

	/**
	 * @brief Constants for how to do the reset when using resetReceiver
	 */
	enum class ResetMode {
		HARDWARE_RESET_IMMEDIATE = 0x00,				//!< Hardware reset of the GPS receiver immediately
		CONTROLLED_SOFTWARE_RESET = 0x01,				//!< Controlled software reset (default)
		CONTROLLED_SOFTWARE_RESET_GNSS_ONLY = 0x02,		//!< Controlled software reset of the GNSS only
		HARDWARE_RESET_AFTER_SHUTDOWN = 0x04,			//!< Hardware reset after doing a software shutdown
		CONTROLLED_GNSS_STOP = 0x08,					//!< Controlled stop of the GNSS
		CONTROLLED_GNSS_START = 0x09					//!< Controlled start of the GNSS
	};

	/**
	 * @brief Resets the GPS. Note: This often does not work.
	 * 
	 * @param startType whether to to a HOT, WARM, or COLD start
	 * 
	 * @param resetMode how to do the reset when using resetReceiver (default: CONTROLLED_SOFTWARE_RESET)
	 * 
	 * In theory you could use this to cold start the GPS for more easily testing things like AssistNow
	 * but in reality, cold booting seems to have no effect.
	 * 
	 */
	void resetReceiver(StartType startType, ResetMode resetMode = ResetMode::CONTROLLED_SOFTWARE_RESET);

	/**
	 * @brief Get the singleton instance of this class
	 */
	static Ublox *getInstance() { return instance; };

protected:
	UbloxCommand<100> incomingCommand;
	
	std::deque<UbloxCommandBase *> commandsToHandle; 
	std::vector<UbloxMessageHandler*> handlers;  	//!< Vector of message handler objects, contains filter and callback function to handle incoming messages
	std::vector<UbloxMessageHandler*> handlersToAdd; 

	static Ublox *instance;	//!< Singleton instance of this class 
};


class AssistNowDownload; // Forward declaration

/**
 * @brief Class to use u-blox AssistNow to get a faster GPS fix 
 * 
 * Using this feature requires several things, described in greater detail in the documentation:
 * - u-blox access token (you can get this at no charge)
 * - Google API token (you can probably use the free tier)
 * - Particle Google Maps Integration to map cell tower ID to location
 * - Particle Integration with Google Elevation API to get elevation from location
 * 
 * As the data will be downloaded by cellular, this will increase your cellular data usage while decreasing
 * the time to first fix. For more details, see the AssistNow.md file in the Github repository for
 * more details.
 * 
 * You typically instantiate this object as a global variable in your main application file. You'll need to
 * call the withAssistNowKey(), setup(), and loop() methods. 
 * 
 * You don't need to explicitly start AssistNow. It will automatically start when the cloud connects and 
 * will check the GPS to see if can see satellites. If it can see fewer than 3 satellites it will 
 */

class UbloxAssistNow {
public:
	/**
	 * @brief Constructor. You typically instantiate one of these as a global variable if you want to use AssistNow
	 * 
	 * This object has minimal storage requirements so it's OK to allocate it all of the time. It will determine
	 * if the GPS needs aiding data and request it automatically. The download temporary buffers are allocated only
	 * when needed and freed when done.
	 */
	UbloxAssistNow();

	/**
	 * @brief Destructor
	 */
	virtual ~UbloxAssistNow();

	/**
	 * @brief Required! Pass your AssistNow API key in this function
	 * 
	 * @param assistNowKey The access token you received from u-blox for using the AssistNow service.
	 * 
	 * The key looks like "c4Pufq9Wp1WnjXo1aNIN5g" as is not the same as any u-blox username or password you
	 * may have.
	 */
	UbloxAssistNow &withAssistNowKey(const char *assistNowKey) { this->assistNowKey = assistNowKey; return *this; };
	
	/**
	 * @brief Runs AssistNow without location hinting
	 * 
	 * This mode does not require the Google Maps integration or the Elevation integration, however it does not
	 * improve the time to first fix as much, and also increases the data usage. With location hinting, the GPS
	 * can get a fix much more quickly, and the aiding data download is smaller because it can skip the satellite
	 * ephemeris data for satellites that won't be in view of your location. 
	 * 
	 * Whenever possible you should use location hinting (set up the webhooks and don't call this function).
	 */
	UbloxAssistNow &withDisableLocation() { this->disableLocation = true; return *this; };

	/**
	 * @brief Call from main application setup. Required!
	 */
	void setup();

	/**
	 * @brief Call from main application loop. Required!
	 * 
	 * It returns quickly when not doing assist now so you can just call it all the time.
	 */
	void loop();

	/**
	 * @brief Gets the singleton instance of this class, 
	 * 
	 * This class is normally instantiated once in the main application code.
	 */
	static UbloxAssistNow *getInstance() { return instance; };

protected:

	/**
	 * @brief State machine handler for waiting for a Particle cloud connection (internal)
	 * 
	 * In order to use the webhooks there must be a cloud connection. While in disableLocation mode
	 * technically we could start after cellular ready, the difference in time is not sufficient to
	 * warrant that special-case.
	 * 
	 * This state will either: 
	 * 
	 * - Find that there are enough satellites visible (3 or more) and aiding
	 * won't likely help much now and go to the stateDone.
	 * - Get cell tower (or Wi-Fi) information and start a Google maps geolocation.
	 * 
	 * Next state: stateWaitLocation or stateWaitDone
	 */
	void stateWaitConnected();

	/**
	 * @brief State machine handler for waiting for location data (internal)
	 * 
	 * Waits for location and elevation data. The subscriptionHandler will move to
	 * stateSendRequest if we get location and elevation.
	 * 
	 * If a timeout occurs (no location information for tower, or webhooks missing)
	 * then it will go to stateWaitDone.
	 * 
	 * Next state: stateSendRequest or stateWaitDone.
	 */ 
	void stateWaitLocation();

	/**
	 * @brief State machine handler for sending a request (internal)
	 * 
	 * This state prepares the request to the u-blox aiding data server and makes
	 * the connection. If an error occurs, goes to stateDone, otherwise goes to
	 * stateReadResponse.
	 * 
	 * Next state: stateReadResponse or stateWaitDone.
	 */ 
	void stateSendRequest();

	/**
	 * @brief State machine handler for reading the response from the u-blox server (internal)
	 * 
	 * If all of the data is read successfully, goes to stateSendToGPS. Otherwise, goes to
	 * stateWaitDone. There is no retry on failure, as if there is a cellular connection
	 * probably it's probably just better to let the GPS get its data from satellite.
	 * 
	 * Next state: stateSendToGPS or stateWaitDone.
	 */ 
	void stateReadResponse();

	/**
	 * @brief State machine handler for sending data to the GPS (internal)
	 * 
	 * The stateReadResponse method buffers all of the GPS aiding data before sending it to
	 * the GPS. The data is generally under 3K, and buffering the whole data helps make sure
	 * it's valid and allows it to be sent out in more controlled bursts to the GPS. 
	 */ 
	void stateSendToGPS();

	/**
	 * @brief State machine handler used when done (internal)
	 * 
	 * The done state will delete the AssistNowDownload object if allocated, otherwise it 
	 * does nothing. This state is entered if:
	 * 
	 * - AssistNow is not needed (GPS has a fix)
	 * - The u-blox AssistNow API key has not been set
	 * - An error occurs (can't get geolocation, elevation, can't contact u-blox aiding server)
	 */ 
	void stateDone();

	/**
	 * @brief Particle event subscription handler (internal)
	 * 
	 * This is only activated if there is a need to do AssistNow. If the GPS already has a fix
	 * the subscription handler is not registered.
	 */ 
	void subscriptionHandler(const char *event, const char *data);


	std::function<void(UbloxAssistNow*)> stateHandler = &UbloxAssistNow::stateWaitConnected; //!< State handler function
	unsigned long stateTime = 0;		//!< Time used for state transition timeouts. The millis() value at the start of the wait.
	AssistNowDownload *download = 0;	//!< State data for downloading including the buffer. This is only allocated when we are downloading to save RAM.
	unsigned long packetDelay = 1;		//!< Delay in milliseconds between messages sent to the GPS during stateSendToGPS.
	bool disableLocation = false;		//!< Set to true to disable getting location data. This causes slower time to first sync and larger downloads.
	unsigned long waitLocationTimeoutMs = 10000; //!< Amount of time in milliseconds to wait for the location and elevation data to arrive.

	String assistNowKey;				//!< Assist now API token/key. Required.
	String assistNowServer = "online-live1.services.u-blox.com";	//!< Server to contact for u-blox aiding data

	static UbloxAssistNow *instance;	//!< Singleton instance of this class
};


/**
 * @brief Class to hold things used during download
 * 
 * Since download happens infrequently (only at startup if the GPS needs it, this object is
 * allocated on the heap by UbloxAssistNow when needed and deleted when not.
 */
class AssistNowDownload {
public:	
	AssistNowDownload();			//!< Constructor
	virtual ~AssistNowDownload();	//!< Destructor

	/**
	 * @brief Allocate the buffer to hold temporary data. Required!
	 * 
	 * @param bufferSize The size of the buffer in bytes. This must be large enough to hold the entire 
	 * download of aiding data!
	 * 
	 * The buffer size varies depending on the number of services, which aiding data is requested,
	 * and whether a location is provided. 
	 */
	bool alloc(size_t bufferSize);

protected:
	size_t bufferSize = 0; 		//!< Size of buffer in bytes as determined from alloc()
	GoogleMapsDeviceLocator locator; //!< Used to find location data from cellular or Wi-Fi 
	float lat = 0.0;			//!< Latitude in degrees from geolocation
	float lng = 0.0;			//!< Longitude in degrees from geolocation
	float accuracy = 0.0;		//!< Accuracy radius in meters from geolocation
	float elev = 0.0;			//!< Elevation in meters from mean sea level from elevation API
	TCPClient client;			//!< TCPClient used to contact the u-blox aiding service
	uint8_t *buffer;			//!< Buffer to store data, allocated during alloc()
	size_t bufferOffset = 0;	//!< Offset currently being written to in buffer
	bool inHeader = true;		//!< Set to true if we are processing the HTTP header or false if not
	size_t contentLength = 0;	//!< Content length of the GPS data

	friend class UbloxAssistNow;
};


#endif /* __UBLOXGPS_H */
