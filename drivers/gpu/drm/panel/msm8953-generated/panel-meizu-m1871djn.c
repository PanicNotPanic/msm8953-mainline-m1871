// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct djn {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline struct djn *to_djn(struct drm_panel *panel)
{
	return container_of(panel, struct djn, panel);
}

#define dsi_generic_write_seq(dsi, seq...) do {				\
		static const u8 d[] = { seq };				\
		int ret;						\
		ret = mipi_dsi_generic_write(dsi, d, ARRAY_SIZE(d));	\
		if (ret < 0)						\
			return ret;					\
	} while (0)

static void djn_reset(struct djn *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(12000, 13000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(12000, 13000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(12000, 13000);
}

static int djn_on(struct djn *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret < 0) {
		dev_err(dev, "Failed to set tear on: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	dsi_generic_write_seq(dsi, 0xb0, 0x00);
	dsi_generic_write_seq(dsi, 0xc1,
			      0x04, 0x48, 0x00, 0xff, 0xee, 0x1d, 0xe5, 0xbe,
			      0xd7, 0xdc, 0xc7, 0x9a, 0xdb, 0xff, 0xff, 0xbf,
			      0x5d, 0x63, 0xe0, 0x9a, 0x7b, 0xf7, 0x10, 0x7b,
			      0xdf, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x40, 0x02, 0x22, 0x03, 0x06, 0x05,
			      0x84, 0x00, 0x01, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xcb,
			      0xff, 0xff, 0xff, 0xff, 0x0f, 0x80, 0x01, 0x00,
			      0x18, 0x00, 0x7e, 0x7e, 0xe0, 0xe7, 0x07, 0x00,
			      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb0, 0x03);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}
	msleep(20);

	return 0;
}

static int djn_off(struct djn *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(20);

	dsi_generic_write_seq(dsi, 0xb0, 0x00);
	dsi_generic_write_seq(dsi, 0xc1,
			      0x04, 0x48, 0x00, 0x7f, 0xef, 0xbd, 0xf7, 0xde,
			      0x7b, 0xef, 0xbd, 0xf7, 0xde, 0xff, 0xff, 0xbf,
			      0xf7, 0xde, 0x7b, 0xef, 0xbd, 0xf7, 0xde, 0x7b,
			      0xef, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x40, 0x02, 0x22, 0x03, 0x06, 0x05,
			      0x84, 0x00, 0x01, 0x00, 0x01);
	dsi_generic_write_seq(dsi, 0xcb,
			      0xff, 0xff, 0xff, 0xff, 0x0f, 0xfe, 0x7f, 0xe0,
			      0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xb0, 0x03);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	return 0;
}

static int djn_prepare(struct drm_panel *panel)
{
	struct djn *ctx = to_djn(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	djn_reset(ctx);

	ret = djn_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int djn_unprepare(struct drm_panel *panel)
{
	struct djn *ctx = to_djn(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = djn_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode djn_mode = {
	.clock = (1080 + 52 + 4 + 52) * (1920 + 60 + 2 + 4) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 52,
	.hsync_end = 1080 + 52 + 4,
	.htotal = 1080 + 52 + 4 + 52,
	.vdisplay = 1920,
	.vsync_start = 1920 + 60,
	.vsync_end = 1920 + 60 + 2,
	.vtotal = 1920 + 60 + 2 + 4,
	.width_mm = 68,
	.height_mm = 121,
};

static int djn_get_modes(struct drm_panel *panel,
			 struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &djn_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs djn_panel_funcs = {
	.prepare = djn_prepare,
	.unprepare = djn_unprepare,
	.get_modes = djn_get_modes,
};

static int djn_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct djn *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	drm_panel_init(&ctx->panel, dev, &djn_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		return ret;
	}

	return 0;
}

static void djn_remove(struct mipi_dsi_device *dsi)
{
	struct djn *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id djn_of_match[] = {
	{ .compatible = "meizu,m1871-djn" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, djn_of_match);

static struct mipi_dsi_driver djn_driver = {
	.probe = djn_probe,
	.remove = djn_remove,
	.driver = {
		.name = "panel-djn",
		.of_match_table = djn_of_match,
	},
};
module_mipi_dsi_driver(djn_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("meizu m1871 djn 1080p video mode dsi panel");
MODULE_LICENSE("GPL v2");
