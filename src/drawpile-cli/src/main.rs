use tracing::Level;

use drawpile_cli::converter::*;
use drawpile_cli::indexer::{decode_index, extract_snapshot, index_recording};
use drawpile_cli::renderer::*;

mod flags {
    use super::*;

    xflags::xflags! {
        cmd drawpile-cli {
            /// Prints version information.
            optional -v, --version

            /// Prints help information.
            default cmd help {}

            /// Converts between binary and text formats.
            cmd convert {
                /// Input file
                required input: String
                /// Output file
                optional output: String
                /// Output format
                optional -f, --format format: Format
            }

            /// Renders a recording.
            cmd render {
                /// Input file
                required input: String
                /// Output file
                optional output: String
                /// Save image every n messages
                optional -e, --every-msg every_msg: u32
                /// Save image every n undo points
                optional --every-up every_up: u32
                /// Resize canvas to this size (WxH)
                optional --resize resize: Size
                /// Resize subsequent images to original size
                optional -s, --same-size same_size: bool
            }

            /// Builds an index for a recording.
            cmd index {
                /// Input file
                required input: String
            }

            /// Decodes a recording index.
            cmd decode-index {
                /// Input file
                required input: String

                /// Extract snapshot at the given index
                optional -x, --extract extract: usize
            }
        }
    }

    impl DrawpileCli {
        pub fn exit_help() -> ! {
            println!("{}", Self::HELP_);
            std::process::exit(2);
        }
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::fmt::Subscriber::builder()
        .with_max_level(Level::DEBUG)
        .init();

    let cli = flags::DrawpileCli::from_env_or_exit();

    if cli.version {
        println!(concat!(
            env!("CARGO_PKG_NAME"),
            " ",
            env!("CARGO_PKG_VERSION")
        ));
        return Ok(());
    }

    match cli.subcommand {
        flags::DrawpileCliCmd::Help(_) => {
            flags::DrawpileCli::exit_help();
        }
        flags::DrawpileCliCmd::Convert(flags::Convert {
            input,
            output,
            format,
        }) => {
            let opts = ConvertRecOpts {
                input_file: &input,
                output_file: output.as_deref().unwrap_or("-"),
                output_format: format.unwrap_or_default(),
            };

            convert_recording(&opts)
        }
        flags::DrawpileCliCmd::Render(flags::Render {
            input,
            output,
            every_msg,
            every_up,
            resize,
            same_size,
        }) => {
            let opts = RenderOpts {
                input_file: &input,
                output_file: output.as_deref().unwrap_or_default(),
                output_every: every_msg.or(every_up),
                every_up: every_up.is_some(),
                resize,
                same_size: same_size.unwrap_or_default(),
            };

            render_recording(&opts)
        }
        flags::DrawpileCliCmd::Index(flags::Index { input }) => index_recording(&input),
        flags::DrawpileCliCmd::DecodeIndex(flags::DecodeIndex { input, extract }) => {
            if let Some(idx) = extract {
                extract_snapshot(&input, idx)
            } else {
                decode_index(&input)
            }
        }
    }
}
