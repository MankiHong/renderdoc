/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include <functional>
#include "core/core.h"
#include "serialise/streamio.h"
#include "strings/string_utils.h"
#include "android.h"

#if ENABLED(RDOC_WIN32)

#include <stdlib.h>
#define endianswap16(x) _byteswap_ushort(x)
#define endianswap32(x) _byteswap_ulong(x)
#define endianswap64(x) _byteswap_uint64(x)

#elif ENABLED(RDOC_APPLE)

#include <libkern/OSByteOrder.h>
#define endianswap16(x) OSSwapInt16(x)
#define endianswap32(x) OSSwapInt32(x)
#define endianswap64(x) OSSwapInt64(x)

#else

#define endianswap16(x) __builtin_bswap16(x)
#define endianswap32(x) __builtin_bswap32(x)
#define endianswap64(x) __builtin_bswap64(x)

#endif

inline uint64_t EndianSwap(uint64_t t)
{
  return endianswap64(t);
}

inline uint32_t EndianSwap(uint32_t t)
{
  return endianswap32(t);
}

inline uint16_t EndianSwap(uint16_t t)
{
  return endianswap16(t);
}

inline int64_t EndianSwap(int64_t t)
{
  return (int64_t)EndianSwap(uint64_t(t));
}

inline int32_t EndianSwap(int32_t t)
{
  return (int32_t)EndianSwap(uint32_t(t));
}

inline int16_t EndianSwap(int16_t t)
{
  return (int16_t)EndianSwap(uint16_t(t));
}

inline double EndianSwap(double t)
{
  uint64_t u;
  memcpy(&u, &t, sizeof(t));
  u = EndianSwap(u);
  memcpy(&t, &u, sizeof(t));
  return t;
}

inline float EndianSwap(float t)
{
  uint32_t u;
  memcpy(&u, &t, sizeof(t));
  u = EndianSwap(u);
  memcpy(&t, &u, sizeof(t));
  return t;
}

inline char EndianSwap(char t)
{
  return t;
}

inline byte EndianSwap(byte t)
{
  return t;
}

inline bool EndianSwap(bool t)
{
  return t;
}

namespace JDWP
{
enum class CommandSet : byte
{
  Unknown = 0,
  VirtualMachine = 1,
  ReferenceType = 2,
  ClassType = 3,
  ArrayType = 4,
  InterfaceType = 5,
  Method = 6,
  Field = 8,
  ObjectReference = 9,
  StringReference = 10,
  ThreadReference = 11,
  ThreadGroupReference = 12,
  ArrayReference = 13,
  ClassLoaderReference = 14,
  EventRequest = 15,
  StackFrame = 16,
  ClassObjectReference = 17,
  Event = 64,
};

enum class TypeTag : byte
{
  Class = 1,
  Interface = 2,
  Arrary = 3,
};

enum class Tag : byte
{
  Array = '[',
  Byte = 'B',
  Char = 'C',
  Object = 'L',
  Float = 'F',
  Double = 'D',
  Int = 'I',
  Long = 'J',
  Short = 'S',
  Void = 'V',
  Boolean = 'Z',
  String = 's',
  Thread = 't',
  ThreadGroup = 'g',
  ClassLoader = 'l',
  ClassObject = 'c',
};

enum class EventKind : byte
{
  SingleStep = 1,
  Breakpoint = 2,
  FramePop = 3,
  Exception = 4,
  UserDefined = 5,
  ThreadStart = 6,
  ThreadDeath = 7,
  ThreadEnd = 7,
  ClassPrepare = 8,
  ClassUnload = 9,
  ClassLoad = 10,
  FieldAccess = 20,
  FieldModification = 21,
  ExceptionCatch = 30,
  MethodEntry = 40,
  MethodExit = 41,
  MethodExitWithReturnValue = 42,
  MonitorContendedEnter = 43,
  MonitorContendedEntered = 44,
  MonitorWait = 45,
  MonitorWaited = 46,
  VMStart = 90,
  VMInit = 90,
  VMDeath = 99,
  VMDisconnected = 100,
};

enum class ModifierKind
{
  Count = 1,
  Conditional = 2,
  ThreadOnly = 3,
  ClassOnly = 4,
  ClassMatch = 5,
  ClassExclude = 6,
  LocationOnly = 7,
  ExceptionOnly = 8,
  FieldOnly = 9,
  Step = 10,
  InstanceOnly = 11,
  SourceNameMatch = 12,
};

enum class SuspendPolicy : byte
{
  None = 0,
  EventThread = 1,
  All = 2,
};

enum class InvokeOptions : int32_t
{
  SingleThreaded = 0x1,
  NonVirtual = 0x2,
};

enum class ClassStatus : int32_t
{
  Verified = 0x1,
  Prepared = 0x2,
  Initialized = 0x4,
  Error = 0x8,
};

enum class Error : uint16_t
{
  None = 0,
  InvalidThread = 10,
  InvalidThreadGroup = 11,
  InvalidPriority = 12,
  ThreadNotSuspended = 13,
  ThreadSuspended = 14,
  ThreadNotAlive = 15,
  InvalidObject = 20,
  InvalidClass = 21,
  ClassNotPrepared = 22,
  InvalidMethodid = 23,
  InvalidLocation = 24,
  InvalidFieldid = 25,
  InvalidFrameid = 30,
  NoMoreFrames = 31,
  OpaqueFrame = 32,
  NotCurrentFrame = 33,
  TypeMismatch = 34,
  InvalidSlot = 35,
  Duplicate = 40,
  NotFound = 41,
  InvalidMonitor = 50,
  NotMonitorOwner = 51,
  Interrupt = 52,
  InvalidClassFormat = 60,
  CircularClassDefinition = 61,
  FailsVerification = 62,
  AddMethodNotImplemented = 63,
  SchemaChangeNotImplemented = 64,
  InvalidTypestate = 65,
  HierarchyChangeNotImplemented = 66,
  DeleteMethodNotImplemented = 67,
  UnsupportedVersion = 68,
  NamesDontMatch = 69,
  ClassModifiersChangeNotImplemented = 70,
  MethodModifiersChangeNotImplemented = 71,
  NotImplemented = 99,
  NullPointer = 100,
  AbsentInformation = 101,
  InvalidEventType = 102,
  IllegalArgument = 103,
  OutOfMemory = 110,
  AccessDenied = 111,
  VmDead = 112,
  Internal = 113,
  UnattachedThread = 115,
  InvalidTag = 500,
  AlreadyInvoking = 502,
  InvalidIndex = 503,
  InvalidLength = 504,
  InvalidString = 506,
  InvalidClassLoader = 507,
  InvalidArray = 508,
  TransportLoad = 509,
  TransportInit = 510,
  NativeMethod = 511,
  InvalidCount = 512,
};

struct CommandData;

struct objectID
{
public:
  operator uint64_t() const { return size == 4 ? data.u32 : data.u64; }
  static int32_t getSize() { return size; }
  static void setSize(int32_t s)
  {
    RDCASSERT(s == 4 || s == 8);
    size = s;
  }

  void EndianSwap()
  {
    if(size == 4)
      data.u32 = ::EndianSwap(data.u32);
    else
      data.u64 = ::EndianSwap(data.u64);
  }

private:
  union
  {
    uint32_t u32;
    uint64_t u64;
  } data;
  static int32_t size;
};

typedef objectID threadID;
typedef objectID threadGroupID;
typedef objectID stringID;
typedef objectID classLoaderID;
typedef objectID classObjectID;
typedef objectID arrayID;
typedef objectID referenceTypeID;
typedef objectID classID;
typedef objectID interfaceID;
typedef objectID arrayTypeID;
typedef objectID methodID;
typedef objectID fieldID;
typedef objectID frameID;

struct taggedObjectID
{
  Tag tag;
  objectID id;
};

struct value
{
  Tag tag;

  union
  {
    arrayID Array;
    byte Byte;
    char Char;
    objectID Object;
    float Float;
    double Double;
    int32_t Int;
    int64_t Long;
    int16_t Short;
    bool Bool;
    objectID String;
    threadID Thread;
    threadGroupID ThreadGroup;
    classLoaderID ClassLoader;
    classObjectID ClassObject;
  };
};

int32_t objectID::size = 8;

struct Location
{
  TypeTag tag;
  objectID classID;
  objectID methodID;
  uint64_t index;
};

struct Command
{
  Command(CommandSet s = CommandSet::Unknown, byte c = 0) : commandset(s), command(c) {}
  void Send(StreamWriter &writer)
  {
    id = idalloc++;
    length = 11 + (uint32_t)data.size();

    uint32_t tmp = EndianSwap(length);
    writer.Write(tmp);
    tmp = EndianSwap(id);
    writer.Write(tmp);

    // no need to endian swap these
    byte flags = 0;
    writer.Write(flags);
    writer.Write(commandset);
    writer.Write(command);

    // already endian swapped
    writer.Write(data.data(), data.size());

    writer.Flush();
  }

  void Recv(StreamReader &reader)
  {
    reader.Read(length);
    length = EndianSwap(length);
    reader.Read(id);
    id = EndianSwap(id);

    byte flags = 0;
    reader.Read(flags);
    if(flags == 0x80)
    {
      commandset = CommandSet::Unknown;
      command = 0;
      uint16_t errInt = 0;
      reader.Read(errInt);
      error = (Error)EndianSwap(errInt);
    }
    else
    {
      reader.Read(commandset);
      reader.Read(command);
      error = Error::None;
    }

    data.resize(length - 11);
    reader.Read(&data[0], data.size());
  }

  // only these need to be publicly writeable
  CommandSet commandset;
  byte command;
  // get the ID assigned/received
  uint32_t GetID() { return id; }
  // get the error received (only for replies)
  Error GetError() { return error; }
  // read-write access to the data (encodes and endian swaps)
  CommandData GetData();

private:
  static uint32_t idalloc;

  // automatically calculated
  uint32_t length = 0;
  uint32_t id = 0;
  Error error = Error::None;
  std::vector<byte> data;
};

struct CommandData
{
  CommandData(std::vector<byte> &dataStore) : data(dataStore) {}
  template <typename T>
  void Read(T &val)
  {
    RDCCOMPILE_ASSERT(sizeof(bool) == 1, "bool must be 1 byte for default template");
    ReadBytes(&val, sizeof(T));
    val = EndianSwap(val);
  }

  template <typename T>
  void Write(const T &val)
  {
    T tmp = EndianSwap(val);
    WriteBytes(&tmp, sizeof(T));
  }

  void Done() { RDCASSERT(offs == data.size()); }
private:
  std::vector<byte> &data;
  size_t offs = 0;

  void ReadBytes(void *bytes, size_t length)
  {
    if(offs + length <= data.size())
    {
      memcpy(bytes, data.data() + offs, length);
      offs += length;
    }
  }
  void WriteBytes(const void *bytes, size_t length)
  {
    const byte *start = (const byte *)bytes;
    data.insert(data.end(), start, start + length);
  }
};

CommandData Command::GetData()
{
  return CommandData(data);
}

template <>
void CommandData::Read(std::string &str)
{
  uint32_t length = 0;
  Read(length);
  str.resize(length);
  ReadBytes(&str[0], length);
}

template <>
void CommandData::Write(const std::string &str)
{
  uint32_t length = (uint32_t)str.size();
  Write(length);
  WriteBytes(str.c_str(), str.size());
}

template <>
void CommandData::Read(objectID &id)
{
  ReadBytes(&id, objectID::getSize());
  id.EndianSwap();
}

template <>
void CommandData::Write(const objectID &id)
{
  objectID tmp = id;
  tmp.EndianSwap();
  WriteBytes(&tmp, objectID::getSize());
}

template <>
void CommandData::Read(taggedObjectID &id)
{
  Read((byte &)id.tag);
  Read(id.id);
}

template <>
void CommandData::Write(const taggedObjectID &id)
{
  Write((const byte &)id.tag);
  Write(id.id);
}

template <>
void CommandData::Read(value &val)
{
  Read((byte &)val.tag);
  switch(val.tag)
  {
    case Tag::Array: Read(val.Array); break;
    case Tag::Byte: Read(val.Byte); break;
    case Tag::Char: Read(val.Char); break;
    case Tag::Object: Read(val.Object); break;
    case Tag::Float: Read(val.Float); break;
    case Tag::Double: Read(val.Double); break;
    case Tag::Int: Read(val.Int); break;
    case Tag::Long: Read(val.Long); break;
    case Tag::Short: Read(val.Short); break;
    case Tag::Void: break;
    case Tag::Boolean: Read(val.Bool); break;
    case Tag::String: Read(val.String); break;
    case Tag::Thread: Read(val.Thread); break;
    case Tag::ThreadGroup: Read(val.ThreadGroup); break;
    case Tag::ClassLoader: Read(val.ClassLoader); break;
    case Tag::ClassObject: Read(val.ClassObject); break;
  }
}

template <>
void CommandData::Write(const value &val)
{
  Write((const byte &)val.tag);
  switch(val.tag)
  {
    case Tag::Array: Write(val.Array); break;
    case Tag::Byte: Write(val.Byte); break;
    case Tag::Char: Write(val.Char); break;
    case Tag::Object: Write(val.Object); break;
    case Tag::Float: Write(val.Float); break;
    case Tag::Double: Write(val.Double); break;
    case Tag::Int: Write(val.Int); break;
    case Tag::Long: Write(val.Long); break;
    case Tag::Short: Write(val.Short); break;
    case Tag::Void: break;
    case Tag::Boolean: Write(val.Bool); break;
    case Tag::String: Write(val.String); break;
    case Tag::Thread: Write(val.Thread); break;
    case Tag::ThreadGroup: Write(val.ThreadGroup); break;
    case Tag::ClassLoader: Write(val.ClassLoader); break;
    case Tag::ClassObject: Write(val.ClassObject); break;
  }
}

template <>
void CommandData::Read(Location &loc)
{
  Read((byte &)loc.tag);
  Read(loc.classID);
  Read(loc.methodID);
  Read(loc.index);
}

template <>
void CommandData::Write(const Location &loc)
{
  Write((const byte &)loc.tag);
  Write(loc.classID);
  Write(loc.methodID);
  Write(loc.index);
}

uint32_t Command::idalloc = 42;

template <CommandSet CmdSet, byte CmdCode>
struct CommandImpl
{
public:
  uint32_t Send(StreamWriter &writer)
  {
    Command cmd(CmdSet, CmdCode);
    FillRequest(cmd.GetData());
    cmd.Send(writer);
    return cmd.GetID();
  }

  void Process(Command &reply) { ProcessReply(reply.GetData()); }
  // handy function when the VM is suspended and you know you can just send a command and receive
  // the reply next thing
  void SendAndProcess(StreamWriter &writer, StreamReader &reader)
  {
    uint32_t id = Send(writer);

    Command reply;
    reply.Recv(reader);
    RDCASSERT(id == reply.GetID());

    ProcessReply(reply.GetData());
  }

protected:
  virtual void ProcessReply(CommandData cmdData) = 0;
  virtual void FillRequest(CommandData cmdData) = 0;
};

struct IDSizesCommand : public CommandImpl<CommandSet::VirtualMachine, 7>
{
public:
  // request data
  /* none */;

  // reply data
  int32_t fieldIDSize;
  int32_t methodIDSize;
  int32_t objectIDSize;
  int32_t referenceTypeIDSize;
  int32_t frameIDSize;

protected:
  void FillRequest(CommandData cmdData) override {}
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read(fieldIDSize);
    cmdData.Read(methodIDSize);
    cmdData.Read(objectIDSize);
    cmdData.Read(referenceTypeIDSize);
    cmdData.Read(frameIDSize);

    // we only support sizes that are all the same
    RDCASSERT(fieldIDSize == methodIDSize && fieldIDSize == objectIDSize &&
              fieldIDSize == referenceTypeIDSize && fieldIDSize == frameIDSize);

    // we also only support 4 or 8 bytes
    RDCASSERT(fieldIDSize == 4 || fieldIDSize == 8);

    objectID::setSize(fieldIDSize);

    cmdData.Done();
  }
};

struct FieldsCommand : public CommandImpl<CommandSet::ReferenceType, 4>
{
  // request data
  referenceTypeID refType;

  // reply data
  struct Field
  {
    methodID id;
    std::string name;
    std::string signature;
    int32_t modBits;
  };
  std::vector<Field> fields;

protected:
  void FillRequest(CommandData cmdData) override { cmdData.Write(refType); }
  void ProcessReply(CommandData cmdData) override
  {
    int32_t numFields = 0;
    cmdData.Read(numFields);

    for(int32_t i = 0; i < numFields; i++)
    {
      Field f;
      cmdData.Read(f.id);
      cmdData.Read(f.name);
      cmdData.Read(f.signature);
      cmdData.Read(f.modBits);

      fields.push_back(f);
    }

    cmdData.Done();
  }
};

struct MethodsCommand : public CommandImpl<CommandSet::ReferenceType, 5>
{
  // request data
  referenceTypeID refType;

  // reply data
  struct Method
  {
    methodID id;
    std::string name;
    std::string signature;
    int32_t modBits;
  };
  std::vector<Method> methods;

protected:
  void FillRequest(CommandData cmdData) override { cmdData.Write(refType); }
  void ProcessReply(CommandData cmdData) override
  {
    int32_t numMethods = 0;
    cmdData.Read(numMethods);

    for(int32_t i = 0; i < numMethods; i++)
    {
      Method m;
      cmdData.Read(m.id);
      cmdData.Read(m.name);
      cmdData.Read(m.signature);
      cmdData.Read(m.modBits);

      methods.push_back(m);
    }

    cmdData.Done();
  }
};

struct VariableTableCommand : public CommandImpl<CommandSet::Method, 2>
{
  // request data
  referenceTypeID refType;
  methodID method;

  // reply data
  int argumentCount = 0;
  struct Slot
  {
    uint64_t codeIndex;
    std::string name;
    std::string signature;
    uint32_t length;
    int32_t slot;
  };
  std::vector<Slot> slots;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(refType);
    cmdData.Write(method);
  }
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read(argumentCount);

    int32_t numSlots = 0;
    cmdData.Read(numSlots);

    for(int32_t i = 0; i < numSlots; i++)
    {
      Slot s;
      cmdData.Read(s.codeIndex);
      cmdData.Read(s.name);
      cmdData.Read(s.signature);
      cmdData.Read(s.length);
      cmdData.Read(s.slot);

      slots.push_back(s);
    }

    cmdData.Done();
  }
};

struct SuspendCommand : public CommandImpl<CommandSet::VirtualMachine, 8>
{
  // no request or reply data
protected:
  void FillRequest(CommandData cmdData) override {}
  void ProcessReply(CommandData cmdData) override { cmdData.Done(); }
};

struct ResumeCommand : public CommandImpl<CommandSet::VirtualMachine, 9>
{
  // no request or reply data
protected:
  void FillRequest(CommandData cmdData) override {}
  void ProcessReply(CommandData cmdData) override { cmdData.Done(); }
};

struct InvokeMethodCommand : public CommandImpl<CommandSet::ObjectReference, 6>
{
  // request data
  objectID object;
  threadID thread;
  classID clazz;
  methodID method;
  std::vector<value> arguments;
  InvokeOptions options = InvokeOptions::SingleThreaded;

  // reply data
  value returnValue;
  taggedObjectID exception;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(object);
    cmdData.Write(thread);
    cmdData.Write(clazz);
    cmdData.Write(method);

    int32_t numArgs = (int32_t)arguments.size();
    cmdData.Write(numArgs);
    for(size_t i = 0; i < arguments.size(); i++)
      cmdData.Write(arguments[i]);

    cmdData.Write((int32_t)options);
  }
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read(returnValue);
    cmdData.Read(exception);

    cmdData.Done();
  }
};
struct InvokeStaticMethodCommand : public CommandImpl<CommandSet::ClassType, 3>
{
  // request data
  classID clazz;
  threadID thread;
  methodID method;
  std::vector<value> arguments;
  InvokeOptions options = InvokeOptions::SingleThreaded;

  // reply data
  value returnValue;
  taggedObjectID exception;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(clazz);
    cmdData.Write(thread);
    cmdData.Write(method);

    int32_t numArgs = (int32_t)arguments.size();
    cmdData.Write(numArgs);
    for(size_t i = 0; i < arguments.size(); i++)
      cmdData.Write(arguments[i]);

    cmdData.Write((int32_t)options);
  }
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read(returnValue);
    cmdData.Read(exception);

    cmdData.Done();
  }
};

struct StackCommand : public CommandImpl<CommandSet::ThreadReference, 6>
{
  StackCommand(threadID t) : thread(t) {}
  // request data
  threadID thread;

  // reply data
  struct Frame
  {
    frameID id;
    Location location;
  };
  std::vector<Frame> frames;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(thread);
    // always fetch full stack
    cmdData.Write<int32_t>(0);
    cmdData.Write<int32_t>(-1);
  }
  void ProcessReply(CommandData cmdData) override
  {
    int32_t numFrames = 0;
    cmdData.Read(numFrames);

    for(int32_t i = 0; i < numFrames; i++)
    {
      Frame f;
      cmdData.Read(f.id);
      cmdData.Read(f.location);

      frames.push_back(f);
    }

    cmdData.Done();
  }
};

struct GetValuesCommand : public CommandImpl<CommandSet::StackFrame, 1>
{
  GetValuesCommand(threadID t, frameID f) : thread(t), frame(f) {}
  // request data
  threadID thread;
  frameID frame;
  struct Slot
  {
    int32_t slot;
    Tag sigbyte;
  };
  std::vector<Slot> slots;

  // reply data
  std::vector<value> values;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(thread);
    cmdData.Write(frame);

    int32_t numSlots = (int32_t)slots.size();
    cmdData.Write(numSlots);
    for(size_t i = 0; i < slots.size(); i++)
    {
      cmdData.Write(slots[i].slot);
      cmdData.Write((byte &)slots[i].sigbyte);
    }
  }
  void ProcessReply(CommandData cmdData) override
  {
    int32_t numValues = 0;
    cmdData.Read(numValues);

    for(int32_t i = 0; i < numValues; i++)
    {
      value v;
      cmdData.Read(v);

      values.push_back(v);
    }

    cmdData.Done();
  }
};

struct SetValuesCommand : public CommandImpl<CommandSet::StackFrame, 2>
{
  SetValuesCommand(threadID t, frameID f) : thread(t), frame(f) {}
  // request data
  threadID thread;
  frameID frame;
  struct Slot
  {
    int32_t slot;
    value val;
  };
  std::vector<Slot> slots;

  // reply data
  /* none */;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write(thread);
    cmdData.Write(frame);

    int32_t numSlots = (int32_t)slots.size();
    cmdData.Write(numSlots);
    for(size_t i = 0; i < slots.size(); i++)
    {
      cmdData.Write(slots[i].slot);
      cmdData.Write(slots[i].val);
    }
  }
  void ProcessReply(CommandData cmdData) override { cmdData.Done(); }
};

struct EventSetCommand : public CommandImpl<CommandSet::EventRequest, 1>
{
  // request data
  EventKind eventKind;
  SuspendPolicy suspendPolicy;
  struct EventModifier
  {
    ModifierKind modKind;

    referenceTypeID ClassOnly;
  };

  std::vector<EventModifier> modifiers;

  EventSetCommand(EventKind k, SuspendPolicy s, const std::vector<EventModifier> &mod)
      : eventKind(k), suspendPolicy(s), modifiers(mod)
  {
  }

  // reply data
  int32_t requestID;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write((const byte &)eventKind);
    cmdData.Write((const byte &)suspendPolicy);

    int32_t numModifiers = (int32_t)modifiers.size();
    cmdData.Write(numModifiers);

    for(const EventModifier &mod : modifiers)
    {
      cmdData.Write((const byte &)mod.modKind);

      RDCASSERT(mod.modKind == ModifierKind::ClassOnly);

      if(mod.modKind == ModifierKind::ClassOnly)
        cmdData.Write(mod.ClassOnly);
    }
  }
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read(requestID);
    cmdData.Done();
  }
};

struct EventClearCommand : public CommandImpl<CommandSet::EventRequest, 2>
{
  // request data
  EventKind eventKind;
  int32_t requestID;

  // reply data
  /* none */;

protected:
  void FillRequest(CommandData cmdData) override
  {
    cmdData.Write((const byte &)eventKind);
    cmdData.Write(requestID);
  }
  void ProcessReply(CommandData cmdData) override { cmdData.Done(); }
};

struct EventCommand : public CommandImpl<CommandSet::Event, 100>
{
public:
  EventCommand(StreamReader &reader)
  {
    Command msg;
    msg.Recv(reader);
    Process(msg);
  }

  // request data
  /* none */;

  struct EventData
  {
    EventKind eventKind;
    int32_t requestID;

    struct
    {
      threadID thread;
      Location location;
    } MethodEntry;

    struct
    {
      threadID thread;
      TypeTag refTypeTag;
      referenceTypeID typeID;
      std::string signature;
      ClassStatus status;
    } ClassPrepare;
  };

  // reply data
  SuspendPolicy suspendPolicy;
  std::vector<EventData> events;

protected:
  void FillRequest(CommandData cmdData) override {}
  void ProcessReply(CommandData cmdData) override
  {
    cmdData.Read((byte &)suspendPolicy);

    int32_t numEvents = 0;
    cmdData.Read(numEvents);

    for(int32_t i = 0; i < numEvents; i++)
    {
      EventData ev;
      cmdData.Read((byte &)ev.eventKind);
      cmdData.Read(ev.requestID);

      switch(ev.eventKind)
      {
        case EventKind::ClassPrepare:
        {
          cmdData.Read(ev.ClassPrepare.thread);
          cmdData.Read((byte &)ev.ClassPrepare.refTypeTag);
          cmdData.Read(ev.ClassPrepare.typeID);
          cmdData.Read(ev.ClassPrepare.signature);
          int32_t dummy = 0;
          cmdData.Read(dummy);
          ev.ClassPrepare.status = (ClassStatus)dummy;
          break;
        }
        case EventKind::MethodEntry:
        {
          cmdData.Read(ev.MethodEntry.thread);
          cmdData.Read(ev.MethodEntry.location);
          break;
        }
        default: RDCERR("Unexpected event! Add handling");
      }

      events.push_back(ev);
    }

    cmdData.Done();
  }
};

referenceTypeID GetType(StreamWriter &writer, StreamReader &reader, const std::string &signature)
{
  Command get(CommandSet::VirtualMachine, 2);
  get.GetData().Write(signature);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  CommandData cmdData = reply.GetData();
  int32_t numTypes = 0;
  cmdData.Read(numTypes);
  if(numTypes == 0)
    return {};

  if(numTypes > 1)
    RDCWARN("Multiple types found for '%s'", signature.c_str());

  byte typetag;
  cmdData.Read(typetag);

  referenceTypeID ret;
  cmdData.Read(ret);
  return ret;
}

referenceTypeID GetObjectType(StreamWriter &writer, StreamReader &reader, objectID obj)
{
  Command get(CommandSet::ObjectReference, 1);
  get.GetData().Write(obj);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  byte tag = 0;
  CommandData cmdData = reply.GetData();
  cmdData.Read(tag);

  referenceTypeID ret = {};
  cmdData.Read(ret);
  return ret;
}

classID GetSuper(StreamWriter &writer, StreamReader &reader, classID clazz)
{
  Command get(CommandSet::ClassType, 1);
  get.GetData().Write(clazz);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  classID ret;
  reply.GetData().Read(ret);
  return ret;
}

std::string GetSignature(StreamWriter &writer, StreamReader &reader, referenceTypeID typeID)
{
  Command get(CommandSet::ReferenceType, 1);
  get.GetData().Write(typeID);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  std::string ret;
  reply.GetData().Read(ret);
  return ret;
}

std::string GetString(StreamWriter &writer, StreamReader &reader, objectID str)
{
  Command get(CommandSet::StringReference, 1);
  get.GetData().Write(str);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  std::string ret;
  reply.GetData().Read(ret);
  return ret;
}

objectID GetThis(StreamWriter &writer, StreamReader &reader, threadID thread, frameID frame)
{
  Command get(CommandSet::StackFrame, 3);
  CommandData cmdData = get.GetData();
  cmdData.Write(thread);
  cmdData.Write(frame);
  get.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(get.GetID() == reply.GetID());

  taggedObjectID ret = {};
  reply.GetData().Read(ret);
  return ret.id;
}

value NewString(StreamWriter &writer, StreamReader &reader, threadID thread, std::string str)
{
  Command newstring(CommandSet::VirtualMachine, 11);
  newstring.GetData().Write(str);
  newstring.Send(writer);

  Command reply;
  reply.Recv(reader);
  RDCASSERT(newstring.GetID() == reply.GetID());

  stringID ret;
  reply.GetData().Read(ret);
  value retvalue;
  retvalue.tag = Tag::String;
  retvalue.String = ret;
  return retvalue;
}

void WaitForEvent(EventSetCommand evSet, StreamWriter &writer, StreamReader &reader,
                  std::function<bool(const EventCommand::EventData &evData)> callback)
{
  evSet.SendAndProcess(writer, reader);

  // now resume execution
  ResumeCommand().SendAndProcess(writer, reader);

  // wait for method entry on the method we care about
  while(!reader.IsErrored())
  {
    EventCommand evCmd(reader);

    for(const EventCommand::EventData &event : evCmd.events)
    {
      if(event.eventKind == evSet.eventKind)
      {
        if(event.requestID == evSet.requestID)
        {
          if(callback(event))
          {
            // stop listening to this event, and leave VM suspended
            EventClearCommand evClear;
            evClear.eventKind = EventKind::MethodEntry;
            evClear.requestID = evSet.requestID;

            evClear.SendAndProcess(writer, reader);
            return;
          }
        }
      }
    }

    // resume to get the next event
    ResumeCommand().SendAndProcess(writer, reader);
  }
}

bool InjectLibraries(Network::Socket *sock, std::string libPath)
{
  StreamWriter writer(sock, Ownership::Nothing);
  StreamReader reader(sock, Ownership::Nothing);

  const int handshakeLength = 14;
  const char handshake[handshakeLength + 1] = "JDWP-Handshake";

  writer.Write(handshake, handshakeLength);
  writer.Flush();

  char response[15] = {};
  reader.Read(response, handshakeLength);

  if(memcmp(handshake, response, 14))
  {
    RDCERR("handshake failed - received >%s< - expected >%s<", response, handshake);
    return false;
  }

  // immediately re-suspend, as connecting will have woken it up
  SuspendCommand().SendAndProcess(writer, reader);

  {
    IDSizesCommand idsizes;
    uint32_t id = idsizes.Send(writer);

    Command reply;
    reply.Recv(reader);
    RDCASSERT(id == reply.GetID());

    idsizes.Process(reply);
  }

  referenceTypeID buildClass = GetType(writer, reader, "Landroid/os/Build;");

  std::string abi = "?";

  {
    FieldsCommand fields;
    fields.refType = buildClass;
    fields.SendAndProcess(writer, reader);

    for(const FieldsCommand::Field &f : fields.fields)
    {
      if(f.name == "CPU_ABI")
      {
        Command get(CommandSet::ReferenceType, 6);
        CommandData cmdData = get.GetData();
        cmdData.Write(buildClass);
        cmdData.Write<int32_t>(1);
        cmdData.Write(f.id);
        get.Send(writer);

        Command reply;
        reply.Recv(reader);
        RDCASSERT(get.GetID() == reply.GetID());

        int32_t numVals = 0;
        value val;
        CommandData cmdData2 = reply.GetData();
        cmdData2.Read(numVals);
        cmdData2.Read(val);

        abi = GetString(writer, reader, val.String);
      }
    }
  }

  if(abi == "armeabi-v7a")
  {
    libPath += "arm";
  }
  else if(abi == "arm64-v8a")
  {
    libPath += "arm64";
  }
  else
  {
    libPath += abi;
  }

  referenceTypeID stringClass = GetType(writer, reader, "Ljava/lang/String;");

  methodID stringConcat = {};

  {
    MethodsCommand meth;
    meth.refType = stringClass;

    meth.SendAndProcess(writer, reader);

    for(const MethodsCommand::Method &m : meth.methods)
    {
      if(m.name == "concat")
        stringConcat = m.id;
    }
  }

  RDCASSERT(stringClass);
  RDCASSERT(stringConcat);

  referenceTypeID vulkanLoaderClass = GetType(writer, reader, "Landroid/app/ApplicationLoaders;");

  if(vulkanLoaderClass)
  {
    // See:
    // https://android.googlesource.com/platform/frameworks/base/+/f9419f0f8524da4980726e06130a80e0fb226763/core/java/android/app/ApplicationLoaders.java
    // for the public getClassLoader.

    methodID vulkanLoaderMethod = {};

    {
      MethodsCommand meth;
      meth.refType = vulkanLoaderClass;

      meth.SendAndProcess(writer, reader);

      for(const MethodsCommand::Method &m : meth.methods)
      {
        if(m.name == "getClassLoader")
        {
          // look for both signatures, not that the final "String classLoaderName" was added
          // recently
          if(m.signature ==
                 "(Ljava/lang/String;IZLjava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)"
                 "Ljava/lang/ClassLoader;" ||
             m.signature ==
                 "(Ljava/lang/String;IZLjava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;"
                 "Ljava/lang/String;)Ljava/lang/ClassLoader;")
          {
            vulkanLoaderMethod = m.id;
            break;
          }
        }
      }
    }

    if(vulkanLoaderMethod == 0)
    {
      RDCERR("Couldn't find getClassLoader method in android.app.ApplicationLoaders");
    }
    else
    {
      // get the variable table (if possible - needs debug symbols)
      VariableTableCommand varTable;
      varTable.refType = vulkanLoaderClass;
      varTable.method = vulkanLoaderMethod;

      varTable.SendAndProcess(writer, reader);

      // default, 4th argument (0 is this)
      int32_t slotIdx = 4;

      for(const VariableTableCommand::Slot &s : varTable.slots)
      {
        if(s.name == "librarySearchPath")
        {
          slotIdx = s.slot;
          break;
        }
      }

      // wait for method entry - WaitForEvent will resume and callback each time this event fires
      EventSetCommand evSet(EventKind::MethodEntry, SuspendPolicy::All,
                            {{ModifierKind::ClassOnly, vulkanLoaderClass}});

      WaitForEvent(evSet, writer, reader,
                   [&writer, &reader, vulkanLoaderMethod, slotIdx, stringClass, stringConcat,
                    libPath](const EventCommand::EventData &evData) -> bool {

                     // skip any events on methods we don't care about
                     if(evData.MethodEntry.location.methodID != vulkanLoaderMethod)
                       return false;

                     threadID thread = evData.MethodEntry.thread;

                     // get the callstack frames
                     StackCommand stack(thread);
                     stack.SendAndProcess(writer, reader);

                     if(stack.frames.empty())
                     {
                       RDCERR("Couldn't get callstack!");
                       return true;
                     }

                     // get the locals on the top (current) frame
                     GetValuesCommand getvals(thread, stack.frames[0].id);
                     getvals.slots = {{slotIdx, Tag::Object}};
                     getvals.SendAndProcess(writer, reader);

                     if(getvals.values.empty() || getvals.values[0].tag != Tag::String)
                     {
                       RDCERR("Couldn't get String local parameter!");
                       return true;
                     }

                     RDCDEBUG("librarySearchPath is %s",
                              GetString(writer, reader, getvals.values[0].String).c_str());

                     value appendSearch = NewString(writer, reader, thread, ":" + libPath);

                     // temp = librarySearchPath.concat(appendSearch);
                     InvokeMethodCommand invoke;
                     invoke.object = getvals.values[0].Object;
                     invoke.thread = thread;
                     invoke.clazz = stringClass;
                     invoke.method = stringConcat;
                     invoke.arguments = {appendSearch};

                     invoke.SendAndProcess(writer, reader);

                     if(invoke.returnValue.tag != Tag::String || invoke.exception.id != 0)
                     {
                       RDCERR("Failed to concat search path!");
                       return true;
                     }

                     RDCDEBUG("librarySearchPath is now %s",
                              GetString(writer, reader, invoke.returnValue.String).c_str());

                     // we will have resume the thread above to call concat, invalidating our
                     // frames.
                     // Re-fetch the callstack
                     stack.frames.clear();
                     stack.SendAndProcess(writer, reader);

                     if(stack.frames.empty())
                     {
                       RDCERR("Couldn't re-fetch callstack!");
                       return true;
                     }

                     // replace the search path with our modified one
                     // librarySearchPath = temp
                     SetValuesCommand setvals(thread, stack.frames[0].id);
                     setvals.slots = {{slotIdx, invoke.returnValue}};
                     setvals.SendAndProcess(writer, reader);

                     return true;
                   });
    }
  }
  else
  {
    RDCERR("Couldn't find class android.app.ApplicationLoaders. Vulkan won't be hooked.");
  }

  // we get here whether we processed vulkan or not.

  referenceTypeID androidApp = GetType(writer, reader, "Landroid/app/Application;");

  if(androidApp == 0)
  {
    RDCERR("Couldn't find android.app.Application");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  methodID appConstruct = {};
  methodID firstOnCreate = {};

  {
    MethodsCommand meth;
    meth.refType = androidApp;

    meth.SendAndProcess(writer, reader);

    for(const MethodsCommand::Method &m : meth.methods)
    {
      if(m.name == "<init>" && m.signature == "()V")
        appConstruct = m.id;

      if(m.name == "onCreate" && m.signature == "()V")
        firstOnCreate = m.id;
    }
  }

  if(appConstruct == 0)
  {
    RDCERR("Couldn't find android.app.Application constructor");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  threadID thread;

  // wait until we hit the constructor
  {
    EventSetCommand appConstructEvent(EventKind::MethodEntry, SuspendPolicy::All,
                                      {{ModifierKind::ClassOnly, androidApp}});
    WaitForEvent(appConstructEvent, writer, reader,
                 [appConstruct, &thread](const EventCommand::EventData &evData) -> bool {
                   thread = evData.MethodEntry.thread;
                   // skip any events on methods we don't care about
                   return (evData.MethodEntry.location.methodID == appConstruct);
                 });
  }

  // get the callstack frames
  StackCommand stack(thread);
  stack.SendAndProcess(writer, reader);

  if(stack.frames.empty())
  {
    RDCERR("Couldn't get callstack!");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  objectID thisPtr = GetThis(writer, reader, thread, stack.frames[0].id);

  if(thisPtr == 0)
  {
    RDCERR("Couldn't find this");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  referenceTypeID thisType = GetObjectType(writer, reader, thisPtr);

  if(thisType == 0)
  {
    RDCERR("Couldn't find this's class");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  methodID getClass = {};
  referenceTypeID searchClass = thisType;

  methodID secondOnCreate = {};

  while(getClass == 0 && searchClass != 0)
  {
    RDCDEBUG("Searching for getClass in '%s'", GetSignature(writer, reader, searchClass).c_str());

    MethodsCommand meth;
    meth.refType = searchClass;

    meth.SendAndProcess(writer, reader);

    for(const MethodsCommand::Method &m : meth.methods)
    {
      if(m.name == "getClass" && m.signature == "()Ljava/lang/Class;")
        getClass = m.id;

      if(m.name == "onCreate" && m.signature == "()V")
        secondOnCreate = m.id;
    }

    if(getClass)
      RDCDEBUG("Found!");

    searchClass = GetSuper(writer, reader, searchClass);
  }

  if(getClass == 0)
  {
    RDCERR("Couldn't find this.getClass()");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  objectID thisClass = {};

  {
    InvokeMethodCommand invoke;
    invoke.object = thisPtr;
    invoke.thread = thread;
    invoke.clazz = thisType;
    invoke.method = getClass;

    invoke.SendAndProcess(writer, reader);

    if(invoke.returnValue.tag != Tag::ClassObject || invoke.exception.id != 0)
    {
      RDCERR("Failed to call this.getClass()!");
      ResumeCommand().SendAndProcess(writer, reader);
      return false;
    }

    thisClass = invoke.returnValue.Object;
  }

  methodID onCreate = {};

  {
    MethodsCommand meth;
    meth.refType = thisClass;

    meth.SendAndProcess(writer, reader);

    for(const MethodsCommand::Method &m : meth.methods)
    {
      if(m.name == "onCreate" && m.signature == "()V")
        onCreate = m.id;
    }
  }

  if(onCreate == 0)
  {
    RDCERR("Couldn't find this.getClass().onCreate()");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  RDCDEBUG("Got final onCreate: %llu (first two were %llu and %llu)", onCreate, firstOnCreate,
           secondOnCreate);

  // wait until we hit onCreate
  {
    EventSetCommand onCreateEvent(EventKind::MethodEntry, SuspendPolicy::All,
                                  {{ModifierKind::ClassOnly, thisClass}});
    WaitForEvent(onCreateEvent, writer, reader,
                 [onCreate, &thread](const EventCommand::EventData &evData) -> bool {
                   thread = evData.MethodEntry.thread;
                   // skip any events on methods we don't care about
                   return (evData.MethodEntry.location.methodID == onCreate);
                 });
  }

  referenceTypeID runtime = GetType(writer, reader, "Ljava/lang/Runtime;");

  if(runtime == 0)
  {
    RDCERR("Couldn't find java.lang.Runtime");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  methodID getRuntime = {};
  methodID load = {};

  {
    MethodsCommand meth;
    meth.refType = runtime;

    meth.SendAndProcess(writer, reader);

    for(const MethodsCommand::Method &m : meth.methods)
    {
      if(m.name == "getRuntime" && m.signature == "()Ljava/lang/Runtime;")
        getRuntime = m.id;
      if(m.name == "load" && m.signature == "(Ljava/lang/String;)V")
        load = m.id;
    }
  }

  if(getRuntime == 0)
  {
    RDCERR("Couldn't find java.lang.Runtime.getRuntime()");
    ResumeCommand().SendAndProcess(writer, reader);
    return false;
  }

  objectID runtimeObject = {};

  {
    InvokeStaticMethodCommand invoke;
    invoke.clazz = runtime;
    invoke.thread = thread;
    invoke.method = getRuntime;

    invoke.SendAndProcess(writer, reader);

    if(invoke.returnValue.tag != Tag::Object || invoke.exception.id != 0)
    {
      RDCERR("Failed to call getClass!");
      ResumeCommand().SendAndProcess(writer, reader);
      return false;
    }

    runtimeObject = invoke.returnValue.Object;
  }

  {
    InvokeMethodCommand invoke;
    invoke.object = runtimeObject;
    invoke.thread = thread;
    invoke.clazz = runtime;
    invoke.method = load;
    invoke.arguments = {
        NewString(writer, reader, thread, libPath + "/libVkLayer_GLES_RenderDoc.so")};

    invoke.SendAndProcess(writer, reader);

    if(invoke.exception.id != 0)
    {
      RDCERR("Failed to call load()!");
      ResumeCommand().SendAndProcess(writer, reader);
      return false;
    }
  }

  // resume before we disconnect
  ResumeCommand().SendAndProcess(writer, reader);

  return true;
}

};    // namespace JDWP

namespace Android
{
bool InjectWithJDWP(uint16_t jdwpport)
{
  Network::Socket *sock = Network::CreateClientSocket("localhost", jdwpport, 500);

  if(sock)
  {
    std::string apk = adbExecCommand("", "shell pm path org.renderdoc.renderdoccmd").strStdout;
    apk = trim(apk);
    apk.resize(apk.size() - 8);
    apk.erase(0, 8);

    bool ret = JDWP::InjectLibraries(sock, apk + "lib/");
    delete sock;

    return ret;
  }
  else
  {
    RDCERR("Couldn't make JDWP connection");
  }

  return false;
}
};    // namespace Android
