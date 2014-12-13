#include <errno.h>
#include <node.h>
#include <nan.h>
#include "./i2c-dev.h"
#include "./readworddata.h"

static int ReadWordData(int fd, int cmd) {
  return i2c_smbus_read_word_data(fd, cmd);
}

class ReadWordDataWorker : public NanAsyncWorker {
public:
  ReadWordDataWorker(NanCallback *callback, int fd, int cmd)
    : NanAsyncWorker(callback), fd(fd), cmd(cmd) {}
  ~ReadWordDataWorker() {}

  void Execute() {
    int ret = ReadWordData(fd, cmd);
    if (ret == -1) {
      SetErrorMessage(strerror(errno));
    } else {
      val = ret;
    }
  }

  void HandleOKCallback() {
    NanScope();

    v8::Local<v8::Value> argv[] = {
      NanNull(),
      NanNew<v8::Integer>(val)
    };

    callback->Call(2, argv);
  }

private:
  int fd;
  int cmd;
  int val;
};

NAN_METHOD(ReadWordDataAsync) {
  NanScope();

  if (args.Length() < 3 || !args[0]->IsInt32() || !args[1]->IsInt32() || !args[2]->IsFunction()) {
    return NanThrowError("incorrect arguments passed to readWordData(int fd, int cmd, function cb)");
  }

  int fd = args[0]->Int32Value();
  int cmd = args[1]->Int32Value();
  NanCallback *callback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new ReadWordDataWorker(callback, fd, cmd));
  NanReturnUndefined();
}

NAN_METHOD(ReadWordDataSync) {
  NanScope();

  if (args.Length() < 2 || !args[0]->IsInt32() || !args[1]->IsInt32()) {
    return NanThrowError("incorrect arguments passed to readWordDataSync(int fd, int cmd)");
  }

  int fd = args[0]->Int32Value();
  int cmd = args[1]->Int32Value();

  int ret = ReadWordData(fd, cmd);
  if (ret == -1) {
    return NanThrowError(strerror(errno), errno);
  }

  NanReturnValue(NanNew<v8::Integer>(ret));
}

