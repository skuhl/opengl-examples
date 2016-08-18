#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void sensorfuse(float corrected[16], const float drifting[16], const float stable[16]);

#ifdef __cplusplus
}
#endif
