idf_component_register(SRCS "bt/bt_main.cpp" "bt/bt_app_av.cpp" "bt/bt_app_core.cpp"
                            "fft/fft_engine.cpp" "fft/fft.cpp"
                            "led/led.cpp"
                            "peripherals/peripherals.cpp" "peripherals/rtc_ds3231.cpp"
                            "common/apis.cpp"
                            "main.cpp"
                    INCLUDE_DIRS "." "./bt" "./led" "./common" "./fft" "./peripherals")
