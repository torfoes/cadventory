import argparse
import logging
import sys
from typing import List, Tuple

import clip
import numpy as np
import torch
from PIL import Image

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler("clip_demo.log")
    ]
)

# select device: use MPS if available (for macOS with Apple Silicon), else CUDA if available, else CPU
if torch.backends.mps.is_available():
    DEVICE = "mps"
elif torch.cuda.is_available():
    DEVICE = "cuda"
else:
    DEVICE = "cpu"

def load_model(model_name: str = "ViT-L/14@336px") -> Tuple[clip.model.CLIP, callable]:
    """
    Load the CLIP model and preprocessing function.

    Args:
        model_name (str): Name of the CLIP model to load.

    Returns:
        Tuple containing the CLIP model and the preprocessing function.
    """
    logging.info(f"Loading CLIP model '{model_name}' on device '{DEVICE}'...")
    try:
        model, preprocess = clip.load(model_name, device=DEVICE)
        model.eval()
        logging.info("CLIP model loaded successfully.")
        return model, preprocess
    except Exception as e:
        logging.error(f"Failed to load CLIP model: {e}")
        sys.exit(1)

def load_image(image_path: str, preprocess) -> torch.Tensor:
    """
    Load and preprocess the image.

    Args:
        image_path (str): Path to the image file.
        preprocess (callable): Preprocessing function from CLIP.

    Returns:
        Preprocessed image tensor.
    """
    try:
        image = Image.open(image_path).convert("RGB")
        logging.info(f"Image '{image_path}' loaded successfully.")
        image_input = preprocess(image).unsqueeze(0).to(DEVICE)
        return image_input
    except Exception as e:
        logging.error(f"Failed to load image '{image_path}': {e}")
        sys.exit(1)

def compute_similarity(
    model: clip.model.CLIP,
    image_input: torch.Tensor,
    labels: List[str]
) -> List[Tuple[str, float]]:
    """
    Compute similarity scores between the image and each label.

    Args:
        model (clip.model.CLIP): Loaded CLIP model.
        image_input (torch.Tensor): Preprocessed image tensor.
        labels (List[str]): List of labels to classify against.

    Returns:
        List of tuples containing labels and their corresponding similarity probabilities.
    """
    try:
        text = clip.tokenize(labels).to(DEVICE)

        with torch.no_grad():
            image_features = model.encode_image(image_input)
            text_features = model.encode_text(text)

            image_features /= image_features.norm(dim=-1, keepdim=True)
            text_features /= text_features.norm(dim=-1, keepdim=True)

            similarity = (image_features @ text_features.T).squeeze(0)

            probs = similarity.softmax(dim=0).cpu().numpy()

        logging.debug(f"Similarity probabilities: {probs}")
        return list(zip(labels, probs))
    except Exception as e:
        logging.error(f"Error during similarity computation: {e}")
        sys.exit(1)

def get_top_labels(
    similarities: List[Tuple[str, float]],
    top_n: int = 5,
    threshold: float = 0.1
) -> List[Tuple[str, float]]:
    """
    Retrieve top N labels above the specified threshold.

    Args:
        similarities (List[Tuple[str, float]]): List of labels and their similarity scores.
        top_n (int): Number of top labels to return.
        threshold (float): Probability threshold for filtering labels.

    Returns:
        List of top labels that meet or exceed the threshold.
    """
    sorted_labels = sorted(similarities, key=lambda x: x[1], reverse=True)
    filtered_labels = [label for label in sorted_labels if label[1] >= threshold]
    top_labels = filtered_labels[:top_n]
    logging.info(f"Top {len(top_labels)} labels above threshold {threshold}: {top_labels}")
    return top_labels

def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Returns:
        Parsed arguments namespace.
    """
    parser = argparse.ArgumentParser(
        description="CLIP Vision Classifier: Classify images based on semantic labels."
    )
    parser.add_argument(
        'image_path',
        type=str,
        help='Path to the image file to classify.'
    )
    parser.add_argument(
        'labels',
        type=str,
        nargs='+',
        help='List of labels to classify the image against.'
    )
    parser.add_argument(
        '--top-n',
        type=int,
        default=5,
        help='Number of top labels to return (default: 5).'
    )
    parser.add_argument(
        '--threshold',
        type=float,
        default=0.1,
        help='Probability threshold for labels (default: 0.1).'
    )
    return parser.parse_args()

def main():
    # parse command-line arguments
    args = parse_arguments()

    if not args.labels:
        logging.error("No labels provided for classification.")
        sys.exit(1)

    model, preprocess = load_model()

    image_input = load_image(args.image_path, preprocess)

    # compute similarity scores between image and labels
    similarities = compute_similarity(model, image_input, args.labels)

    top_labels = get_top_labels(similarities, top_n=args.top_n, threshold=args.threshold)

    print("\n--- Classification Results ---")
    if top_labels:
        for label, prob in top_labels:
            print(f"Label: {label}, Probability: {prob:.4f}")
    else:
        print("No labels exceeded the specified threshold.")

if __name__ == '__main__':
    main()
