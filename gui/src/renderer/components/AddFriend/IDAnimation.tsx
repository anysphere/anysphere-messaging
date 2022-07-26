//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

import seedrandom from "seedrandom";
import { motion } from "framer-motion";
import { useState, useEffect, useMemo } from "react";
import { classNames } from "../../utils";
import { sha256string, randint } from "./utils";

const DEBUG_COLORS = false;
// const DEBUG_COLORS = true;

// IDAnimation is used to provide a visual version of the public ID string.
// It has the following requirements:
// 1. It has high entropy.
// 2. Two different public IDs have different-looking animations.
// 3. It is easy to see if two pictures are the same.
// 4. There is both a still image version and an animation version, where the two are oblviously related.
// It should not be dependent on the current time. The duration for the entire animation should be < 1 second.
export function IDAnimation({
  seed,
  bordersize,
}: {
  seed: string;
  bordersize: number;
}): JSX.Element {
  // see StoryAnimation for detailed information here
  const [shaseed, setShaseed] = useState("");

  useEffect(() => {
    sha256string(seed)
      .then((hash) => {
        setShaseed(hash);
      })
      .catch((err) => {
        console.error(err);
      });
  }, [seed]);

  // create circles (position, size, color)

  const [circles, background, border] = useMemo(() => {
    const rng = seedrandom(shaseed);
    const n = randint(rng, 8, 12);
    function getColor(hue: number): string[] {
      const saturation = randint(rng, 85, 95);
      const lightness = randint(rng, 50, 70);
      const complementary = randint(rng, 0, 1);
      const hue1 = hue;
      const hue2 = complementary ? (hue + 150) % 360 : (hue + 210) % 360;
      return [
        `hsl(${hue1}, ${saturation}%, ${lightness}%)`,
        `hsl(${hue2}, ${saturation}%, ${lightness}%)`,
      ];
    }
    function getBgColor(hue: number): string {
      const saturation = randint(rng, 85, 95);
      const lightness = randint(rng, 97, 98);
      return `hsl(${hue}, ${saturation}%, ${lightness}%)`;
    }
    function getBorderColor(hue: number): string {
      const saturation = randint(rng, 85, 95);
      const lightness = randint(rng, 17, 23);
      return `hsl(${hue}, ${saturation}%, ${lightness}%)`;
    }
    const hue = randint(rng, 0, 360);
    const circles: {
      position: { x: number; y: number };
      size: number;
      colors: string[];
    }[] = Array(n)
      .fill(0)
      .map((_, j) => {
        return {
          position: {
            x: 50 + randint(rng, -50, 50),
            y: 50 + randint(rng, -50, 50),
          },
          size: randint(rng, 30, 50),
          colors: getColor(hue),
        };
      });
    return [circles, getBgColor(hue), getBorderColor(hue)];
  }, [shaseed]);

  return (
    <>
      <div
        className={classNames(
          "absolute h-full w-full overflow-clip rounded-full",
          DEBUG_COLORS ? "bg-green-100" : ""
        )}
        style={{
          backgroundColor: background,
          willChange: "opacity",
          borderColor: border,
          borderWidth: bordersize + "px",
        }}
      >
        {circles.map((circle, i) => {
          return (
            <div
              style={{
                position: "absolute",
                top: `${circle.position.y - circle.size / 2}%`,
                left: `${circle.position.x - circle.size / 2}%`,
                height: `${circle.size}%`,
                width: `${circle.size}%`,
                background: `linear-gradient(30deg, ${circle.colors[0]}, ${circle.colors[1]})`,
                opacity: 0.5,
              }}
              className={`rounded-full mix-blend-multiply`}
              key={i}
            ></div>
          );
        })}
      </div>
    </>
  );
}
