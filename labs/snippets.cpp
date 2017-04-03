// Update velocitys depending on orbit angle
default_random_engine rand(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
uniform_real_distribution<float> dist;
for (unsigned int i = 0; i < MAX_PARTICLES; ++i) {
	velocitys[i] = vec4(0.0f, 0.1f + (10.0f * dist(rand)) * cosf(rotAngle), 0.1f + (10.0f * dist(rand)) * sinf(rotAngle), 0.0f);
}