int red, green, blue;
int intensity = 255;

void kelvinToRgb(int temperature) {
  temperature = constrain(temperature, 1000, 40000);
  temperature /= 100;
  if (temperature <= 66)
    red = intensity;
  else
    red = (int) round(329.7 * (pow(temperature - 60, -0.13)));
  red = constrain(red, 0, intensity);
  if (temperature <= 66)
    green = (int) round(99.5 * log(temperature) - 161.12);
  else
    green = (int) round(288.1 * (pow(temperature - 60, -0.08)));
  green = constrain(green, 0, intensity);
  if (temperature >= 66)
    blue = intensity;
  else if (temperature <= 19)
    blue = 0;
  else
    blue = (int) round(138.5 * log(temperature - 10) - 305.04);
  blue = constrain(blue, 0, intensity);
}

void nmToRgb(int wavelength) {
  float gamma = 0.80;
  float factor;
  float r, g, b;
  if ((wavelength >= 380.0) && (wavelength < 440.0)) {
    r = -(wavelength - 440.0) / (440.0 - 380.0);
    g = 0.0;
    b = 1.0;
  } else if ((wavelength >= 440.0) && (wavelength < 490.0)) {
    r = 0.0;
    g = (wavelength - 440.0) / (490.0 - 440.0);
    b = 1.0;
  } else if ((wavelength >= 490.0) && (wavelength < 510.0)) {
    r = 0.0;
    g = 1.0;
    b = -(wavelength - 510.0) / (510.0 - 490.0);
  } else if ((wavelength >= 510.0) && (wavelength < 580.0)) {
    r = (wavelength - 510.0) / (580.0 - 510.0);
    g = 1.0;
    b = 0.0;
  } else if ((wavelength >= 580.0) && (wavelength < 645.0)) {
    r = 1.0;
    g = -(wavelength - 645.0) / (645.0 - 580.0);
    b = 0.0;
  } else if ((wavelength >= 645.0) && (wavelength < 781.0)) {
    r = 1.0;
    g = 0.0;
    b = 0.0;
  } else {
    r = 0.0;
    g = 0.0;
    b = 0.0;
  };
  // Let the intensity fall off near the vision limits
  if ((wavelength >= 380.0) && (wavelength < 420.0)) {
    factor = 0.3 + 0.7 * (wavelength - 380.0) / (420.0 - 380.0);
  } else if ((wavelength >= 420.0) && (wavelength < 701.0)) {
    factor = 1.0;
  } else if ((wavelength >= 701.0) && (wavelength < 781.0)) {
    factor = 0.3 + 0.7 * (780.0 - wavelength) / (780.0 - 700.0);
  } else {
    factor = 0.0;
  };
  if (r != 0) {
    r = round(intensity * pow(r * factor, gamma));
  }
  if (g != 0) {
    g = round(intensity * pow(g * factor, gamma));
  }
  if (b != 0) {
    b = round(intensity * pow(b * factor, gamma));
  };
  red = constrain(r, 0, intensity);
  green = constrain(g, 0, intensity);
  blue = constrain(b, 0, intensity);
}
