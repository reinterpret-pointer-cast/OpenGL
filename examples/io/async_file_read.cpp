#include <fan/pch.h>

#include <uv.h>

#include <coroutine>

template <typename T>
struct task;

template <typename T>
struct task_promise {
  T value;
  std::exception_ptr exception;

  task<T> get_return_object();

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }

  void return_value(T v) { value = v; }

  void unhandled_exception() { exception = std::current_exception(); }

  task_promise() = default;
  task_promise(const task_promise&) = delete;
  task_promise& operator=(const task_promise&) = delete;
  task_promise(task_promise&&) = default;
  task_promise& operator=(task_promise&&) = default;
};

template <typename T>
struct task {
  using promise_type = task_promise<T>;
  std::coroutine_handle<promise_type> coro;

  task(std::coroutine_handle<promise_type> h) : coro(h) {}
  ~task() { if (coro) coro.destroy(); }

  task(const task&) = delete;
  task& operator=(const task&) = delete;

  task(task&& other) noexcept : coro(other.coro) {
    other.coro = nullptr;
  }

  task& operator=(task&& other) noexcept {
    if (this != &other) {
      if (coro) {
        coro.destroy();
      }
      coro = other.coro;
      other.coro = nullptr;
    }
    return *this;
  }

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) noexcept { coro.resume(); }
  T await_resume() {
    if (coro.promise().exception) {
      std::rethrow_exception(coro.promise().exception);
    }
    return coro.promise().value;
  }
};

template <typename T>
task<T> task_promise<T>::get_return_object() {
  return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

template <>
struct task_promise<void> {
  std::exception_ptr exception;

  task<void> get_return_object();

  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }

  void return_void() {}

  void unhandled_exception() { exception = std::current_exception(); }

  task_promise() = default;
  task_promise(const task_promise&) = delete;
  task_promise& operator=(const task_promise&) = delete;
  task_promise(task_promise&&) = default;
  task_promise& operator=(task_promise&&) = default;
};

template <>
struct task<void> {
  using promise_type = task_promise<void>;
  std::coroutine_handle<promise_type> coro;

  task() = default;
  task(std::coroutine_handle<promise_type> h) : coro(h) {}
  ~task() { if (coro) coro.destroy(); }

  task(const task&) = delete;
  task& operator=(const task&) = delete;

  task(task&& other) noexcept : coro(other.coro) {
    other.coro = nullptr;
  }

  task& operator=(task&& other) noexcept {
    if (this != &other) {
      if (coro) {
        coro.destroy();
      }
      coro = other.coro;
      other.coro = nullptr;
    }
    return *this;
  }

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) noexcept { coro.resume(); }
  void await_resume() {
    if (coro.promise().exception) {
      std::rethrow_exception(coro.promise().exception);
    }
  }
};

task<void> task_promise<void>::get_return_object() {
  return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)};
}


struct uv_sleep_awaitable {
  uv_timer_t timer_req;
  std::coroutine_handle<> handle;
  task<void>* me_ptr;

  uv_sleep_awaitable(task<void>* me, uv_loop_t* loop, uint64_t delay) : me_ptr(me) {
    timer_req.data = this;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, on_timer_cb, delay, 0);
  }

  static void on_timer_cb(uv_timer_t* timer) {
    auto* self = static_cast<uv_sleep_awaitable*>(timer->data);
    self->handle.resume();
    uv_timer_stop(timer);
    uv_close((uv_handle_t*)timer, nullptr);
    self->me_ptr->coro.resume();
  }

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) noexcept {
    handle = h;
  }
  void await_resume() noexcept {}
};

task<void> co_sleep_for(task<void>* me, uv_loop_t* loop, uint64_t delay) {
  uv_sleep_awaitable sleep_req(me, loop, delay);
  co_await sleep_req;
}

struct uv_fs_awaitable {
  uv_fs_t req;
  task<void>* me_ptr;
  std::coroutine_handle<> handle;

  uv_fs_awaitable(task<void>* me) : me_ptr(me){
    req.data = this;
  }

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) noexcept {
    handle = h;
  }
  void await_resume() noexcept {}
};

struct uv_fs_open_awaitable : uv_fs_awaitable {
  uv_fs_open_awaitable(task<void>* me, const std::string& path, int flags, int mode) : uv_fs_awaitable(me) {
    req.data = this;
    uv_fs_open(uv_default_loop(), &req, path.c_str(), flags, mode, on_open_cb);
  }

  static void on_open_cb(uv_fs_t* r) {
    auto self = static_cast<uv_fs_open_awaitable*>(r->data);
    self->handle.resume();
    self->me_ptr->coro.resume();
  }

  int result() const noexcept { return req.result; }
};

struct uv_fs_read_awaitable : uv_fs_awaitable {
  uv_fs_read_awaitable(task<void>* me, int file, uv_buf_t buf, int64_t offset) : uv_fs_awaitable(me) {
    uv_fs_read(uv_default_loop(), &req, file, &buf, 1, offset, on_read_cb);
  }

  static void on_read_cb(uv_fs_t* req) {
    auto self = static_cast<uv_fs_read_awaitable*>(req->data);
    self->handle.resume();
    self->me_ptr->coro.resume();
  }

  ssize_t result() const noexcept { return req.result; }
};

struct uv_fs_close_awaitable : uv_fs_awaitable {
  uv_fs_close_awaitable(int file) : uv_fs_awaitable(nullptr) {
    uv_fs_close(uv_default_loop(), &req, file, on_close_cb);
  }

  static void on_close_cb(uv_fs_t* req) {
    auto self = static_cast<uv_fs_close_awaitable*>(req->data);
    self->handle.resume();
    //t.coro.resume();
    // doesnt need t.coro resume since this object wont be created like in previous structs
  }
};

task<int> open_file(task<void>* me, const std::string& path) {
  uv_fs_open_awaitable open_req(me, path, O_RDONLY, 0);
  co_await open_req;
  auto r = open_req.result();
  if (r < 0) {
    fan::throw_error("failed to open file:" + path);
  }
  co_return r;
}

task<ssize_t> read_file(task<void>* me, int file, char* buffer, size_t buffer_size, int64_t offset) {
  uv_buf_t buf = uv_buf_init(buffer, buffer_size);
  uv_fs_read_awaitable read_req(me, file, buf, offset);
  co_await read_req;
  auto r = read_req.result();
  if (r < 0) {
    fan::throw_error("error reading file", std::to_string(r));
  }
  co_return r;
}

// broken, how to even make this co_await if it has no cb
task<int64_t> sizeof_file(int file) {
  uv_fs_t stat_req;
  int r = uv_fs_fstat(uv_default_loop(), &stat_req, file, NULL);
  if (r < 0) {
    co_return -1;
  }
  co_return stat_req.statbuf.st_size;
}

task<void> my_function(task<void>* me) {
  try {
    int fd = co_await open_file(me, "1.cpp");

    int offset = 0;
    while (true) {
      char buffer[64];
      ssize_t result = co_await read_file(me, fd, buffer, sizeof(buffer), offset);
      if (result== 0) {
        break;
      }
      else {
        buffer[result] = '\0';
        printf("Read data: %s\n", buffer);
        offset += result;
      }
      co_await co_sleep_for(me, uv_default_loop(), 100);
    }

   // 
    co_await uv_fs_close_awaitable(fd);
  }
  catch (const std::runtime_error& e) {
    
  }
}


void event_run() {
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void register_event_function(task<void>* t, task<void> (* func_ptr)(task<void>*)) {
  *t = func_ptr(t);
  t->coro.resume();
}

// not needed i guess
//void non_blocking_getch() {
//  static uv_tty_t handle;
//
//  uv_tty_init(uv_default_loop(), &handle, 0, 0);
//  uv_read_start((uv_stream_t*)&handle, [](auto ...) {}, [](auto ...) {});
//}

int main() {
  //task<void> t = my_function(&t);
  task<void> t;
  register_event_function(&t, my_function);
  event_run();

  return 0;
}
