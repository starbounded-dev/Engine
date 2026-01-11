#version 460 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_UV;

uniform float u_Time;
uniform vec2  u_Resolution;   // (width, height)
uniform vec2  u_FlameOrigin;  // UV space, e.g. (0.5, 0.05)

float hash(vec2 p)
{
    // cheap, stable hash
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p)
{
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; i++)
    {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

void main()
{
    vec2 uv = v_UV;

    // aspect-corrected space around origin
    float aspect = (u_Resolution.x > 0.0 && u_Resolution.y > 0.0) ? (u_Resolution.x / u_Resolution.y) : 1.0;
    vec2 p = uv - u_FlameOrigin;
    p.x *= aspect;

    // flame rises upward in +Y (uv.y)
    float t = u_Time;

    // turbulence field
    vec2 q = vec2(p.x * 3.0, p.y * 3.0 - t * 1.2);
    float n = fbm(q + vec2(0.0, t * 0.25));

    // “column” shape (narrow near base, wider up)
    float height = clamp((p.y + 0.15) * 1.6, 0.0, 1.0);
    float width  = mix(0.10, 0.55, height);
    float core   = 1.0 - smoothstep(width, width + 0.20, abs(p.x + (n - 0.5) * 0.35));

    // intensity: strongest near origin, fades with height
    float base = smoothstep(0.12, 0.0, length(p));
    float intensity = core * (0.35 + 1.3 * base) * (1.0 - height * 0.65);

    // add flicker “tongues”
    float tongues = smoothstep(0.35, 0.95, fbm(vec2(p.x * 4.0, p.y * 6.0 - t * 2.0)));
    intensity *= mix(0.75, 1.25, tongues);

    // color ramp
    vec3 colDark   = vec3(0.02, 0.02, 0.03);
    vec3 colRed    = vec3(0.85, 0.15, 0.05);
    vec3 colOrange = vec3(1.00, 0.55, 0.10);
    vec3 colYellow = vec3(1.00, 0.95, 0.55);

    float hot = clamp(intensity * 1.2, 0.0, 1.0);
    vec3 col = mix(colDark, colRed, smoothstep(0.05, 0.35, hot));
    col = mix(col, colOrange, smoothstep(0.20, 0.70, hot));
    col = mix(col, colYellow, smoothstep(0.60, 1.00, hot));

    // alpha: if you want transparent background, use intensity here
    fragColor = vec4(col, 1.0);
}
