#include "ui.h"
#include "utils.h"

#include "debug.h"
#include "gettext.h"
#include <deadbeef/deadbeef.h>

#include <gtkmm.h>
#include <regex>
#include <string>
#include <time.h>
#include <vector>

using namespace std;
using namespace Gtk;
using namespace Glib;

struct linessizes {
  int titlesize;
  int artistsize;
  int newlinesize;
};

const DB_playItem_t *last;

struct timespec tss = {0, 100000000};

// TODO: eliminate all the global objects, as their initialization is not well
// defined

static TextView *lyricView;
static ScrolledWindow *lyricbar;
static RefPtr<TextBuffer> refBuffer;
static RefPtr<TextTag> tagItalic, tagBold, tagLarge, tagCenter, tagSmall,
    tagForegroundColorHighlight, tagForegroundColorRegular, tagLeftmargin,
    tagRightmargin, tagRegular;
static vector<RefPtr<TextTag>> tagsTitle, tagsArtist, tagsSyncline,
    tagsNosyncline, tagPadding;

bool isValidHexaCode(string str) {
  regex hexaCode("^#([a-fA-F0-9]{6}|[a-fA-F0-9]{3})$");
  return regex_match(str, hexaCode);
}

// "Size" lyrics to be able to make lines "dissapear" on top.
// "Size" lyrics to be able to make lines "dissapear" on top.
vector<int> sizelines(DB_playItem_t *track, string lyrics) {
  // 添加输入验证
  if (!track || lyrics.empty()) {
    debug_out << "sizelines: Invalid input parameters" << std::endl;
    return vector<int>{0, 0, 0};
  }

  // 检查UI组件是否有效
  if (!lyricView || !refBuffer || !lyricbar) {
    debug_out << "sizelines: UI components not initialized" << std::endl;
    return vector<int>{0, 0, 0};
  }

  set_lyrics(track, lyrics, "", "", "");
  debug_out << "Sizelines started" << "\n";
  death_signal = 1;
  int temporaly = 0;
  vector<int> values;

  // 改进的等待机制：动态等待直到渲染完成
  int wait_attempts = 0;
  const int max_wait_attempts = 20;           // 最多等待200ms
  struct timespec short_wait = {0, 10000000}; // 10ms

  do {
    nanosleep(&short_wait, NULL);
    wait_attempts++;

    // 强制处理挂起的事件
    while (Gtk::Main::events_pending()) {
      Gtk::Main::iteration(false);
    }

  } while (wait_attempts < max_wait_attempts &&
           (!lyricView->get_realized() || refBuffer->get_line_count() <= 1));

  auto allocation = lyricbar->get_allocation();
  if (allocation.get_height() <= 0) {
    debug_out << "sizelines: Invalid lyricbar allocation" << std::endl;
    death_signal = 0;
    return vector<int>{0, 0, 0};
  }

  // 改进视口高度计算，增加缓冲区
  int viewport_height = allocation.get_height() *
                        (deadbeef->conf_get_int("lyricbar.vpostion", 50)) / 100;

  values.push_back(viewport_height);
  values.push_back(0);

  // 验证buffer的行数
  int line_count = refBuffer->get_line_count();
  if (line_count <= 3) {
    debug_out << "sizelines: Insufficient lines in buffer: " << line_count
              << std::endl;
    death_signal = 0;
    return vector<int>{values[0], 0, 0};
  }

  Gdk::Rectangle rectangle;
  vector<int> line_heights; // 存储每行的实际高度

  // 改进的行高计算
  for (int i = 0; i < line_count; i++) {
    try {
      auto iter = refBuffer->get_iter_at_line(i);
      if (!iter) {
        debug_out << "sizelines: Invalid iterator for line: " << i << std::endl;
        continue;
      }

      lyricView->get_iter_location(iter, rectangle);

      if (i == 0) {
        temporaly = rectangle.get_y();
        line_heights.push_back(rectangle.get_height());
      } else {
        int current_y = rectangle.get_y();
        int height_diff = current_y - temporaly;

        // 使用实际行高而不是y差值，提供更准确的计算
        int actual_height = max(height_diff, rectangle.get_height());
        line_heights.push_back(actual_height);

        // 跳过标题和艺术家行（前2行），从歌词开始计算
        if (i >= 2) {
          values.push_back(actual_height);
        }

        temporaly = current_y;
      }

    } catch (const std::exception &e) {
      debug_out << "sizelines: Exception at line " << i << ": " << e.what()
                << std::endl;
      continue;
    } catch (...) {
      debug_out << "sizelines: Unknown exception at line " << i << std::endl;
      continue;
    }
  }

  // 验证values数组
  if (values.size() < 3) {
    debug_out << "sizelines: Insufficient values calculated" << std::endl;
    death_signal = 0;
    return vector<int>{values[0], 0, 0};
  }

  // 改进的可见行数计算
  int visible_lines = 0;
  int accumulated_height = 0;
  int base_viewport = values[0]; // 减去缓冲区得到实际可视高度

  for (size_t i = 2; i < values.size(); i++) {
    accumulated_height += values[i];
    if (accumulated_height <= base_viewport) {
      visible_lines++;
    } else {
      // 部分可见的行也要考虑进去，提供更平滑的滚动
      float partial_visibility =
          (float)(base_viewport - (accumulated_height - values[i])) / values[i];
      if (partial_visibility > 0.3) { // 如果行的30%以上可见，就计算在内
        visible_lines++;
      }
      break;
    }
  }

  values[1] = max(0, visible_lines - 1); // 确保不会为负数

  debug_out << "Sizelines finished: viewport=" << viewport_height
            << ", visible_lines=" << visible_lines
            << ", total_values=" << values.size() << std::endl;

  death_signal = 0;
  return values;
}

void set_lyrics(DB_playItem_t *track, string past, string present,
                string future, string padding) {
  signal_idle().connect_once([track, past, present, future, padding] {
    // debug_out << "set_lyrics past: " << past << std::endl;
    // debug_out << "set_lyrics present: " << present << std::endl;
    // debug_out << "set_lyrics future: " << future << std::endl;
    if (!is_playing(track)) {
      return;
    }
    string artist, title;
    deadbeef->pl_lock();
    artist = deadbeef->pl_find_meta(track, "artist") ?: _("Unknown Artist");
    title = deadbeef->pl_find_meta(track, "title") ?: _("Unknown Title");
    deadbeef->pl_unlock();

    refBuffer->erase(refBuffer->begin(), refBuffer->end());

    refBuffer->insert_with_tags(refBuffer->begin(), title, tagsTitle);

    if (g_utf8_validate(future.c_str(), -1, NULL)) {
      refBuffer->insert_with_tags(refBuffer->end(),
                                  string{"\n"} + artist + "\n\n", tagsArtist);
      refBuffer->insert_with_tags(refBuffer->end(), padding, tagPadding);
      refBuffer->insert_with_tags(refBuffer->end(), past, tagsNosyncline);
      refBuffer->insert_with_tags(refBuffer->end(), present, tagsSyncline);
      refBuffer->insert_with_tags(refBuffer->end(), future, tagsNosyncline);
    } else {
      death_signal = 1;
      string error = "Wrong character encoding";
      refBuffer->insert_with_tags(refBuffer->end(), padding, tagPadding);
      refBuffer->insert_with_tags(refBuffer->end(), error, tagsSyncline);
    }
  });
}

// To have scroll bars or not when lyrics are synced or not.
void sync_or_unsync(bool syncedlyrics) {
  if (syncedlyrics == true) {
    lyricbar->set_policy(POLICY_EXTERNAL, POLICY_EXTERNAL);
  } else {
    lyricbar->set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
  }
}

Justification get_justification() {

  int align = deadbeef->conf_get_int("lyricbar.lyrics.alignment", 1);
  switch (align) {
  case 0:
    return JUSTIFY_LEFT;
  case 2:
    return JUSTIFY_RIGHT;
  default:
    return JUSTIFY_CENTER;
  }
}

void get_tags() {
  tagItalic = refBuffer->create_tag();
  tagItalic->property_style() = Pango::STYLE_ITALIC;
  tagItalic->property_scale() =
      deadbeef->conf_get_float("lyricbar.fontscale", 1);

  tagRegular = refBuffer->create_tag();
  tagRegular->property_scale() =
      deadbeef->conf_get_float("lyricbar.fontscale", 1);

  tagLeftmargin = refBuffer->create_tag();
  tagLeftmargin->property_left_margin() =
      deadbeef->conf_get_float("lyricbar.border", 22) *
      deadbeef->conf_get_float("lyricbar.fontscale", 1);

  tagRightmargin = refBuffer->create_tag();
  tagRightmargin->property_right_margin() =
      deadbeef->conf_get_float("lyricbar.border", 22) *
      deadbeef->conf_get_float("lyricbar.fontscale", 1);

  tagBold = refBuffer->create_tag();
  tagBold->property_scale() = deadbeef->conf_get_float("lyricbar.fontscale", 1);
  tagBold->property_weight() = Pango::WEIGHT_BOLD;

  tagLarge = refBuffer->create_tag();
  tagLarge->property_scale() =
      1.2 * deadbeef->conf_get_float("lyricbar.fontscale", 1);

  tagSmall = refBuffer->create_tag();
  tagSmall->property_scale() = 0.0001;

  tagCenter = refBuffer->create_tag();
  tagCenter->property_justification() = JUSTIFY_CENTER;

  tagForegroundColorHighlight = refBuffer->create_tag();
  tagForegroundColorRegular = refBuffer->create_tag();

  if (isValidHexaCode(
          deadbeef->conf_get_str_fast("lyricbar.highlightcolor", "#571c1c"))) {
    tagForegroundColorHighlight->property_foreground() =
        deadbeef->conf_get_str_fast("lyricbar.highlightcolor", "#571c1c");
  } else {
    tagForegroundColorHighlight->property_foreground() = "#571c1c";
  }

  if (isValidHexaCode(
          deadbeef->conf_get_str_fast("lyricbar.regularcolor", "#000000"))) {
    tagForegroundColorRegular->property_foreground() =
        deadbeef->conf_get_str_fast("lyricbar.regularcolor", "#000000");
  } else {
    tagForegroundColorRegular->property_foreground() = "#000000";
  }

  tagsTitle = {tagLarge, tagBold, tagCenter};
  tagsArtist = {tagItalic, tagCenter};

  tagsSyncline = {tagBold, tagForegroundColorHighlight};

  if (deadbeef->conf_get_int("lyricbar.bold", 1) == 1) {
    tagsSyncline = {tagBold, tagForegroundColorHighlight};
    tagsNosyncline = {tagRegular, tagLeftmargin, tagRightmargin,
                      tagForegroundColorRegular};
  } else {
    tagsSyncline = {tagRegular, tagForegroundColorHighlight};
    tagsNosyncline = {tagRegular, tagForegroundColorRegular};
  }

  if (get_justification() == JUSTIFY_LEFT) {
    tagRightmargin->property_right_margin() =
        deadbeef->conf_get_float("lyricbar.border", 22) *
        deadbeef->conf_get_float("lyricbar.fontscale", 1);
    tagsNosyncline = {tagRegular, tagRightmargin};
  }
  if (get_justification() == JUSTIFY_RIGHT) {
    tagLeftmargin->property_left_margin() =
        deadbeef->conf_get_float("lyricbar.border", 22) *
        deadbeef->conf_get_float("lyricbar.fontscale", 1);
    tagsNosyncline = {tagRegular, tagLeftmargin};
  }

  tagPadding = {tagSmall};
}

extern "C"

    GtkWidget *
    construct_lyricbar() {
  Gtk::Main::init_gtkmm_internals();
  refBuffer = TextBuffer::create();
  get_tags();
  lyricView = new TextView(refBuffer);
  lyricView->set_editable(false);
  lyricView->set_can_focus(false);
  lyricView->set_name("lyricView");
  lyricView->set_left_margin(2);
  lyricView->set_right_margin(2);
  lyricView->set_justification(get_justification());
  lyricView->get_vadjustment()->set_value(1000);
  lyricView->set_wrap_mode(WRAP_WORD_CHAR);
  if (get_justification() == JUSTIFY_LEFT) {
    lyricView->set_left_margin(20);
  }
  lyricView->show();
  lyricbar = new ScrolledWindow();
  lyricbar->add(*lyricView);
  lyricbar->set_name("lyricbar");
  lyricbar->set_policy(POLICY_EXTERNAL, POLICY_EXTERNAL);

  /**********/

  // Create css config text.
  std::string cssconfig("\
  		#lyricView text {\
  		background-color: ");
  if (isValidHexaCode(
          deadbeef->conf_get_str_fast("lyricbar.backgroundcolor", "#F6F6F6"))) {
    cssconfig.append(
        deadbeef->conf_get_str_fast("lyricbar.backgroundcolor", "#F6F6F6"));
  } else {
    cssconfig.append("#F6F6F6");
  }
  cssconfig.append(";\
		}\
	");

  // load css
  auto data = g_strdup_printf("%s", cssconfig.c_str());

  RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
  cssProvider->load_from_data(data);

  RefPtr<Gtk::StyleContext> styleContext = Gtk::StyleContext::create();

  // get default screen
  RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

  // add provider for screen in all application
  styleContext->add_provider_for_screen(
      screen, cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  return GTK_WIDGET(lyricbar->gobj());
}

extern "C" int message_handler(struct ddb_gtkui_widget_s *, uint32_t id,
                               uintptr_t ctx, uint32_t, uint32_t) {
  auto event = reinterpret_cast<ddb_event_track_t *>(ctx);
  switch (id) {
  case DB_EV_CONFIGCHANGED:
    debug_out << "CONFIG CHANGED\n";
    get_tags();
    signal_idle().connect_once(
        [] { lyricView->set_justification(get_justification()); });
    break;
  case DB_EV_TERMINATE:
    debug_out << "DEADBEEF CLOSED \n";
    death_signal = 1;
    break;
    //		case DB_EV_TRACKINFOCHANGED:
    //			debug_out << "TRACKINFOCHANGED" << "\n";
    //			break;
  case DB_EV_PLUGINSLOADED:
  case DB_EV_SONGSTARTED:
    debug_out << "SONG STARTED\n";
    if (!event->track || event->track == last ||
        deadbeef->pl_get_item_duration(event->track) <= 0.0) {
      //				std::cout << "if in" << "\n";
      return 0;
    }
    last = event->track;
    //			std::cout << "SONG STARTED" << "\n";
    auto tid = deadbeef->thread_start(update_lyrics, event->track);
    deadbeef->thread_detach(tid);
    break;
  }

  return 0;
}

extern "C" void lyricbar_destroy() {
  delete lyricbar;
  delete lyricView;
  tagsArtist.clear();
  tagPadding.clear();
  tagsSyncline.clear();
  tagsNosyncline.clear();
  tagRegular.clear();
  tagsTitle.clear();
  tagLarge.reset();
  tagSmall.reset();
  tagBold.reset();
  tagItalic.reset();
  tagRightmargin.reset();
  tagLeftmargin.reset();
  refBuffer.reset();
}
