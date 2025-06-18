const end = 15;

const leds = [];

for (let i = 0; i < 70; i++) {
    leds.push(end - i * 3 / 4);
}

let red = 0;
let green = 0;
let yellow = 0;

for (let i = 0; i < 70; i++) {
    if (leds[i] <= 0) {
        green += 2;
    } else if (leds[i] <= 9) {
        yellow += 2;
    } else {
        red += 2;
    }
}

const levels = [];

for (let i = 0; i < 70; i++) {
    levels.push(Math.round(Math.pow(10, (leds[i] - 15) / 20) * 4095));
    console.info(i, leds[i], levels[levels.length - 1]);
}

console.info('{ ' + levels.reverse().join(", ") + " }");