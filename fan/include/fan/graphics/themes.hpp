#pragma once

#include <fan/types/color.hpp>

namespace fan_2d {
	
	namespace graphics {

		namespace gui {

			namespace defaults {

				inline fan::color text_color(1);
				inline fan::color text_color_place_holder = fan::color::hex(0x757575);
				inline f32_t font_size(32);

			}

			struct animation {

				animation(fan::window* window) : timer(), window_(window) {

				}

				animation(fan::window* window, uint64_t time, std::function<void()> animation_function) : 
					timer(fan::time::nanoseconds(time)),
					m_animation(animation_function),
					window_(window)
				{
					timer.start();
				}
				~animation() {
					this->clear();
				}

				animation(const animation& a) {
					this->operator=(a);
				}
				animation(animation&& a) {
					this->operator=(std::move(a));
				}

				animation& operator=(const animation& a) {

					this->clear();

					this->state = a.state;

					this->timer = a.timer;

					this->window_ = a.window_;

					this->m_animation = a.m_animation;

					this->call_id = -1;

					if (a.call_id != -1) {
						set_animation(a.timer.m_time, this->m_animation);
					}

					return *this;
				}

				animation& operator=(animation&& a) {

					this->state = a.state;

					this->timer = a.timer;

					this->window_ = a.window_;

					this->call_id = a.call_id;

					this->m_animation = a.m_animation;

					a.call_id = -1;

					return *this;
				}

				void clear() {

					if (call_id == -1) {
						return;
					}

					window_->erase_reserved_call(call_id);

					call_id = -1;
				}

				void erase_animation() {
					if (call_id != -1) {
						window_->edit_reserved_call(call_id, this, []{});
					}
				}

				void set_delay_time(uint64_t time) {
					timer = fan::time::clock(fan::time::nanoseconds(time));
					timer.start();
				}

				void set_animation_function(std::function<void()> animation_function) {
					m_animation = animation_function;
				}

				void set_animation(uint64_t time, std::function<void()> animation_function) {
					timer = fan::time::clock(fan::time::nanoseconds(time));
					m_animation = animation_function;

					timer.start();

					if (call_id != (uint32_t)-1) {
						window_->edit_reserved_call(call_id, this, [&] {
							
							m_animation();

							if (timer.started() && timer.finished()) {

								timer.restart();
								state = !state;
							}
						});
					}
					else {
						call_id = window_->push_reserved_call(this, m_animation);
					}

				}

				bool state = 0;

				fan::time::clock timer;

			protected:

				fan::window* window_ = nullptr;

				uint32_t call_id = -1;

				std::function<void()> m_animation;
			};

			struct theme {

				theme(fan::window* window) : button(window) {

				}

				struct button : public animation {

					button(fan::window* window) : animation(window) {

					}

					enum class states_e {
						outside,
						hover,
						click
					};

					fan::color color;
					fan::color outline_color;
					fan::color text_color;

					fan::color hover_color;
					fan::color hover_outline_color;

					fan::color click_color;
					fan::color click_outline_color;

					f32_t outline_thickness;

				}button;

				struct checkbox {

					fan::color color;
					fan::color text_color;

					fan::color hover_color;

					fan::color click_color;

					fan::color check_color;

				}checkbox;

			};

			namespace themes {

				struct empty : public fan_2d::graphics::gui::theme {

					empty(fan::window* window) : theme(window) {

						button.color = fan::color(0, 0, 0);
						button.outline_color = fan::color(0, 0, 0);
						button.text_color = fan_2d::graphics::gui::defaults::text_color;
						button.outline_thickness = 0; // px

						button.hover_color = button.color + 0.1;
						button.hover_outline_color = button.outline_color + 0.1;

						button.click_color = button.hover_color + 0.05;
						button.click_outline_color = button.click_color + 0.05;

						checkbox.color = button.color;
						checkbox.text_color = button.text_color;
						checkbox.hover_color = button.hover_color;
						checkbox.click_color = button.click_color;
						checkbox.check_color = fan::color(1, 1, 1, 0);

					}

				};	

				struct deep_blue : public fan_2d::graphics::gui::theme {

					deep_blue(fan::window* window) : theme(window) {

						button.color = fan::color(0, 0, 0.3);
						button.outline_color = fan::color(0, 0, 0.5);
						button.text_color = fan_2d::graphics::gui::defaults::text_color;
						button.outline_thickness = 2; // px
						
						button.hover_color = button.color + 0.1;
						button.hover_outline_color = button.outline_color + 0.1;
						
						button.click_color = button.hover_color + 0.05;
						button.click_outline_color = button.click_color + 0.05;

						checkbox.color = button.color;
						checkbox.text_color = button.text_color;
						checkbox.hover_color = button.hover_color;
						checkbox.click_color = button.click_color;
						checkbox.check_color = fan::color(1, 1, 1);

					}

				};	

				struct deep_red : public fan_2d::graphics::gui::theme {

					deep_red(fan::window* window) : theme(window) {

						button.color = fan::color(0.3, 0, 0);
						button.outline_color = fan::color(0.5, 0, 0);
						button.text_color = fan_2d::graphics::gui::defaults::text_color;
						button.outline_thickness = 2; // px

						button.hover_color = button.color + 0.1;
						button.hover_outline_color = button.outline_color + 0.1;

						button.click_color = button.hover_color + 0.05;
						button.click_outline_color = button.click_color + 0.05;

						checkbox.color = button.color;
						checkbox.text_color = button.text_color;
						checkbox.hover_color = button.hover_color;
						checkbox.click_color = button.click_color;
						checkbox.check_color = fan::color(0.2, 0, 0);

					}

				};

				struct white : public fan_2d::graphics::gui::theme {

					white(fan::window* window) : theme(window) {

						button.color = fan::color(0.8, 0.8, 0.8);
						button.outline_color = fan::color(0.9, 0.9, 0.9);
						button.text_color = fan_2d::graphics::gui::defaults::text_color;
						button.outline_thickness = 2; // px

						button.hover_color = button.color + 0.1;
						button.hover_outline_color = button.outline_color + 0.1;

						button.click_color = button.hover_color + 0.05;
						button.click_outline_color = button.click_color + 0.05;

						checkbox.color = button.color;
						checkbox.text_color = button.text_color;
						checkbox.hover_color = button.hover_color;
						checkbox.click_color = button.click_color;
						checkbox.check_color = fan::color(0.5, 0, 0);

					}

				};	

				struct locked : public fan_2d::graphics::gui::theme {

					locked(fan::window* window) : theme(window) {

						button.color = fan::color(0.2, 0.2, 0.2);
						button.outline_color = fan::color(0.3, 0.3, 0.3);
						button.text_color = fan_2d::graphics::gui::defaults::text_color;
						button.outline_thickness = 2; // px

						button.hover_color = button.color + 0.1;
						button.hover_outline_color = button.outline_color + 0.1;

						button.click_color = button.hover_color + 0.05;
						button.click_outline_color = button.click_color + 0.05;

						checkbox.color = button.color;
						checkbox.text_color = button.text_color;
						checkbox.hover_color = button.hover_color;
						checkbox.click_color = button.click_color;
						checkbox.check_color = fan::color(0.5, 0, 0);

					}

				};	

			}

		}

	}

}