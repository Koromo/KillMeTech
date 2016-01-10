#ifndef _KILLME_KEYEVENTS_H_
#define _KILLME_KEYEVENTS_H_

#include <string>

namespace killme
{
    namespace keyEvents
    {
        extern const std::string keyPressed;
        extern const std::string keyReleased;
    }

 //   enum class KeyCode;

 //   /** Key pressed event */
 //   class KeyPressed : public Event
 //   {
 //   private:
 //       KeyCode key_;

 //   public:
 //       /** Event type definition */
 //       static const std::string eventType;

 //       /** Construct with event key */
 //       explicit KeyPressed(KeyCode key) : Event(KeyPressed::eventType), key_(key) {}

 //       /** Returns pressed key */
 //       KeyCode getKey() const { return key_; }
 //   };

	///** Key Released event */
	//class KeyReleased : public Event
	//{
	//private:
	//	KeyCode key_;

	//public:
 //       /** Event type definition */
 //       static const std::string eventType;

	//	/** Construct with event key */
	//	explicit KeyReleased(KeyCode key) : Event(KeyReleased::eventType), key_(key) {}

	//	/** Returns pressed key */
 //       KeyCode getKey() const { return key_; }
	//};
}

#endif