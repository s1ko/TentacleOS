#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting LVGL Image Conversion Automation...${NC}"

echo -e "Step 1: Downloading scripts and requirement lists..."
BASE_URL="https://raw.githubusercontent.com/lvgl/lvgl/master/scripts"

wget -q -N "${BASE_URL}/LVGLImage.py"
wget -q -N "${BASE_URL}/prerequisites-apt.txt"
wget -q -N "${BASE_URL}/prerequisites-pip.txt"

echo -e "Step 3: Activating virtual environment..."
if [ -f "lvgl-env/bin/activate" ]; then
  source lvgl-env/bin/activate
else
  echo -e "${RED}Error: Virtual environment 'env' not found.${NC}"
  echo "Please create it first with: python3 -m venv env"
  exit 1
fi

echo -e "Step 4: Converting images to BIN (ARGB8888)..."
TARGET_DIRS=("UI" "frames" "icons" "img" "label")

for dir in "${TARGET_DIRS[@]}"; do
  FULL_PATH="./assets/$dir"

  if [ -d "$FULL_PATH" ]; then
    echo -e "Processing directory: ${GREEN}$FULL_PATH${NC}"

    python3 LVGLImage.py "$FULL_PATH" --ofmt BIN --cf ARGB8888 --align 1 -o "$FULL_PATH"

    if [ $? -eq 0 ]; then
      echo -e "Cleaning up PNG files in $FULL_PATH..."
      find "$FULL_PATH" -name "*.png" -type f -delete
    else
      echo -e "${RED}Conversion failed in $FULL_PATH. Keeping PNGs.${NC}"
    fi
  else
    echo -e "${YELLOW}Warning: Directory '$FULL_PATH' not found. Skipping...${NC}"
  fi
done

echo -e "\n${GREEN}-------------------------------${NC}"
echo -e "${GREEN}      Automation Complete      ${NC}"
echo -e "${GREEN}-------------------------------${NC}"
