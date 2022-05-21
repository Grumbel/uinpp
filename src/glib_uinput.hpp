#ifndef HEADER_GLIB_UINPUT_HPP
#define HEADER_GLIB_UINPUT_HPP

#include <glib.h>

class GlibUInput
{
public:
  GlibUInput(UInput& m_uinput) :
    m_uinput(uinput),
    m_timeout_id(),
    m_timer(g_timer_new())
  {
    // FIXME: hardcoded timeout is kind of evil
    // FIXME: would be nicer if UInput didn't depend on glib
    m_timeout_id = g_timeout_add(
      10,
      [](gpointer data) -> gboolean {
        UInput* const self = static_cast<UInput*>(data);
        int const msec_delta = static_cast<int>(g_timer_elapsed(self->m_timer, NULL) * 1000.0f);
        g_timer_reset(self->m_timer);
        self->update(msec_delta);
        return true; // do not remove the callback
      }, this);
  }

  ~GlibUInput()
  {
    g_source_remove(m_timeout_id);
    g_timer_destroy(m_timer);
  }

public:
  UInput& m_uinput;
  guint m_timeout_id;
  GTimer* m_timer;

public:
  GlibUinput(const GlibUinput&) = delete;
  GlibUinput& operator=(const GlibUinput&) = delete;
};

class GlibLinuxUinput
{
public:
  GlibLinuxUinput(LinuxUinput& linux_uinput) :
    m_linux_uinput(linux_uinput),
    m_io_channel(),
    m_source_id()
  {
    // start g_io_channel
    m_io_channel = g_io_channel_unix_new(m_fd);

    // set encoding to binary
    GError* error = NULL;
    if (g_io_channel_set_encoding(m_io_channel, NULL, &error) != G_IO_STATUS_NORMAL)
    {
      log_error(error->message);
      g_error_free(error);
    }

    g_io_channel_set_buffered(m_io_channel, false);

    m_source_id = g_io_add_watch(m_io_channel,
                                 static_cast<GIOCondition>(G_IO_IN | G_IO_ERR | G_IO_HUP),
                                 [](GIOChannel* source, GIOCondition condition, gpointer userdata) -> gboolean {
                                   static_cast<LinuxUinput*>(userdata)->read();
                                   return TRUE;
                                 }, this);
  }

  ~GlibLinuxUinput()
  {
    g_source_remove(m_source_id);
  }

private:
  LinuxUinput& linux_uinput;
  GIOChannel* m_io_channel;
  guint m_source_id;

public:
  GlibLinuxUinput(const GlibLinuxUinput&) = delete;
  GlibLinuxUinput& operator=(const GlibLinuxUinput&) = delete;
}

#endif

/* EOF */
